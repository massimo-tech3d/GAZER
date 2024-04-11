defmodule Steppers do
  require Logger
  use Agent

  @az_step 12
  @az_dir 19
  @alt_step 13
  @alt_dir 26

  @cw 1    # clockwise
  @ccw -1  # counterclockwise

  # # TBD - TO BE MOVED TO CONFIG FILE
  # @worm             144  # whorm ratio of AZ Mount
  # @big_p             60  # teeth of lerger pulley
  # @small_p           20  # teeth of smaller pulley
  # @motors_deg_step  0.9  # degrees per step for the motors (equal for AZ and ALT)
  # @full_steps_per_deg  @worm * @big_p / @small_p / @motors_deg_step
  # # xx_reverse serve ad invertire la polarità del motore, senza dover invertire il cavo
  # @alt_reverse        1  # alt aumenta --> :cw  alt decresce --> :ccw
  # @az_reverse         1  # az aumenta --> :cw  az decresce --> :ccw


  @moduledoc """
  This module controls the steppers speed without ramping up the speed, because with my telescope weight
  ramping up is not required. The motors are able to jump to X12 speed from zero, even from -X12 speed

  Should the motors stall the Stepper genserver module should be used instead of this, because it keeps
  status of each motor speed/dir and allows to gradually ramp the motors without stopping the current
  movement.
  """

  ## pulse counter GENSERVER IS THE WAY TO GO
  # message handling to be executed in a separate process spawned when counting is needed
  # executes
  # Pigpiox.GPIO.WatcherSupervisor.start_watcher(ALTPIN, self())
  # Pigpiox.GPIO.WatcherSupervisor.start_watcher(AZPIN, self())
  # waits for messages like
  # {:gpio_leveL_change, gpio, level}
  # increments the relative counter

  # starts counting on specific pin and holds the direction
  # calls Pigpiox.GPIO.watch(ALT|AZ) and counts the messages received
  # returns the current count and keeps counting
  # stops counting and resets to zero


  @doc """
  riceve una mappa così:
      motors = %{speed_az: XX, dir_az: X, speed_alt: YY, dir_az: Y}
  se un motore manca, la sua velocità viene azzerata
  """
  def init_motors(conf) do
    Pigpiox.GPIO.set_mode(@az_dir, :output)
    Pigpiox.GPIO.set_mode(@az_step, :output)
    Pigpiox.GPIO.set_mode(@alt_dir, :output)
    Pigpiox.GPIO.set_mode(@alt_step, :output)

    Pigpiox.GPIO.write(4, 1)   # AZ to 32 usteps
    Pigpiox.GPIO.write(3, 0)
    Pigpiox.GPIO.write(2, 1)

    Pigpiox.GPIO.write(22, 1)  # ALT to 32 usteps
    Pigpiox.GPIO.write(27, 0)
    Pigpiox.GPIO.write(17, 1)

    Logger.info("Steppers: storing configuration")
    Agent.start_link(fn -> conf end, name: __MODULE__)
    # launch the pulse counter GenServer here
    Logger.info("Initialized steppers")
  end

  # Configuration functions

  def configuration_change(conf) do
    Agent.update(__MODULE__, fn _state -> conf end)
  end

  def fspd() do
    c = Agent.get(__MODULE__, fn state -> state end)
    %{alt_fspd: c.alt_fspd, az_fspd: c.az_fspd}
  end

  def fspd(id) do
    Logger.info("Steppers: getting fspd #{id}")
    c = Agent.get(__MODULE__, fn state -> state end)
    case id do
      :az  -> c.az_fspd
      :alt -> c.alt_fspd
    end
  end

  def reversals() do
    c = Agent.get(__MODULE__, fn state -> state end)
    %{alt_reverse: c.alr, az_reverse: c.azr}
  end

  def reversal(id) do
    c = Agent.get(__MODULE__, fn state -> state end)
    case id do
      :az -> c.az_r
      :alt -> c.alt_r
    end
  end

  def change_conf(conf) do
    Agent.update(__MODULE__, fn -> conf end)
  end

  # End configuration functions

  def spin(motors) do
    Logger.debug("SPIN CALLED")
    motors |> inspect() |> Logger.debug()
    speed_az = Map.get(motors, :speed_az, 0)
    speed_alt = Map.get(motors, :speed_alt, 0)
    motor_speed(:az, speed_az)
    motor_speed(:alt, speed_alt)
  end

  # TODO -- CAN DELETE AFTER EMI TESTING
  def spin_alt(speed) do
    Logger.debug("spinning alt motor #{speed}")
    motor_speed(:alt, speed)
  end

  def stop_all do
    stop_motor(:az)
    stop_motor(:alt)
    Logger.debug("STOP_ALL EXECUTED")
  end

  def stop_motor(id) when id == :alt do
    Pigpiox.Pwm.hardware_pwm(@alt_step, 0, 100_000)
    Logger.debug("STOPPED ALT MOTOR")
  end

  def stop_motor(id) when id == :az do
    Pigpiox.Pwm.hardware_pwm(@az_step, 0, 100_000)
    Logger.debug("STOPPED AZ MOTOR")
  end

  # receives degrees and speeds per each motor
  # calculates how long time each motor needs to spin and starts them
  def spin_degrees({daz, dalt}, {speed_az, speed_alt}, ramp\\false) do
    # calculates how many seconds each motor will spin
    alt_sec = abs(safe_div(dalt, speed_alt))    # sec < 0 is counterclockwise
    az_sec = abs(safe_div(daz, speed_az))       # sec < 0 is counterclockwise

    # starts the motors and waits for both to finish
    azt  = Task.async(fn -> timed_motor(:az, speed_az, az_sec, ramp) end)
    altt = Task.async(fn -> timed_motor(:alt, speed_alt, alt_sec, ramp) end)
    _tasks_with_results = Task.yield_many([azt, altt], 300*1000)  # timeout 5 minuti
     # Just returns
  end

  defp timed_motor(id, speed, seconds, ramp\\false) do
    if ramp do
      motor_speed(id, speed/3)  # does a slower start - motor may stall otherwise
      :timer.sleep(500)
    end
    motor_speed(id, speed)
    :timer.sleep(round(1000*seconds))
    stop_motor(id)
  end

  # TODO make a nearly continuous acceleration
  # defp timed_motor(id, speed, seconds, ramp\\false) do
  #   if ramp do
  #     motor_speed(id, speed/4)  # does a slower start - motor may stall otherwise
  #     :timer.sleep(200)
  #     motor_speed(id, speed/3)  # does a slower start - motor may stall otherwise
  #     :timer.sleep(200)
  #     motor_speed(id, speed/2)  # does a slower start - motor may stall otherwise
  #     :timer.sleep(200)
  #   end
  #   motor_speed(id, speed)
  #   :timer.sleep(round(1000*seconds))
  #   stop_motor(id)
  # end

  defp motor_speed(id, speed) when id == :az do
    case sign(speed) * reversal(id) do
      1  -> Pigpiox.GPIO.write(@az_dir, 0)
      -1 -> Pigpiox.GPIO.write(@az_dir, 1)
    end
    full_steps_per_deg = fspd(id)
    herz = round(abs(speed * full_steps_per_deg * 32))
    Logger.debug("SPEED AZ HERZ #{herz}")
    Pigpiox.Pwm.hardware_pwm(@az_step, herz, 100_00)
  end
  defp motor_speed(id, speed) when id == :alt do
    case sign(speed) * reversal(id) do
      1  -> Pigpiox.GPIO.write(@alt_dir, 0)
      -1 -> Pigpiox.GPIO.write(@alt_dir, 1)
    end
    full_steps_per_deg = fspd(id)
    herz = round(abs(speed * full_steps_per_deg * 32))
    Logger.debug("SPEED ALT HERZ #{herz}")
    Pigpiox.Pwm.hardware_pwm(@alt_step, herz, 100_00)
  end
  defp motor_speed(id, _speed) do
    :error
  end

  defp sign(f) when f > 0 do
    1
  end
  defp sign(_f) do
    -1
  end

  defp safe_div(_a, b) when b==0 do
    0
  end
  defp safe_div(a, b) do
    a/b
  end
end
