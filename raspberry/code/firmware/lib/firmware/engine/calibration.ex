defmodule Engine.Calibration do
  @moduledoc """
  Calibration is started by direct call to start_calibration which calls calibrate_gyro and sets status to Calibration
  if current status is :Tracking the calibration command is aborted immediately without interrupting tracking

  responds to the external events when in calibration mode. Events can be:
  - Sensors reading
  - Calibration
  - Joystick
  - Joystick
  - GUI

  From :Calibration, the state can change to :Idle (button event aborting calibration or calibration completed)
  """
  require Logger
  alias Engine.Motors
  alias Engine.Aim

  @calispeedslow  1     # 1 gradi/sec - Motors.SlewX4
#  @calispeedfast  3     # 3 gradi/sec - Motors.SlewX12
  @calispeedfast  2     # 2 gradi/sec - Motors.SlewX8

  @doc """
  catches and ignores gui messages.
  """
  def gui(_msg, state) do
    Engine.send_to_gui(state, %{label: "The calibration procedure can't be interrupted", goto_ra: :nok, goto_dec: :nok, goto: false})
  end

  @doc """
  response to joystick event in calibration status
  - button -- aborts calibration
  - all else is ignored
  """
  def joystick(["button"], state) do
    Logger.info("Calibration aborted")
    Motors.stop_motors()
    Engine.send_to_gui(state, "calibration aborted")
    %{state | status: :Idle, m_calibrated: :false}
  end

  def joystick(_data, state) do
    state
  end

  def sensors(payload, state) do
    Aim.aim_change(payload, state)
  end

  @doc """
  handles calibration messages
     :start only in Idle and not done yet, otherwise ignore
     :first_round, only in Calibration status otherwise ignore
     :second_round, only in Calibration status otherwise ignore
     :abort, only in Calibration status otherwise ignore
  """
  def m_calibrate(:start, state) when state.status == :Idle and state.m_calibrated == false do
    Logger.info("CALIBRATION - mcalibrate")
    spawn(Engine.Calibration, :go_vertical, [state, self()])
    %{state | status: :Calibration, manual: :false}
  end

  def m_calibrate(command, state) when state.status == :Calibration do
    case command do
      :first_step ->
        Logger.info("Calibration rotate 360")
        seconds = ceil((360 / @calispeedfast) * 2)  # * 2 perchè fa andata e ritorno
        Logger.debug("720° AZ rotation at speed #{@calispeedfast} will last #{seconds} seconds")
        Circuits.UART.write(state.serial_pid, "m_calibration #{seconds}")  # starts the data collection on the microcontroller side
        spawn(Engine.Calibration, :rotate_360, [state, self()])
        state
      :second_step ->
        Logger.info("Calibration rotate -360")
        spawn(Engine.Calibration, :counterotate_360, [state, self()])
        state
      # :third_step ->   # swing from 90° alt to 0° alt
      #   # communicate to arduino
      #   seconds = ceil((state.aim.pos_alt / @calispeedslow))
      #   spawn(Engine.Calibration, :go_horizontal, [state, self(), seconds])
      #   state
      :abort ->  # this is redundant, abort is catched by joystick button press
        Engine.Motors.stop_motors()
        Engine.send_to_gui(state, "calibration aborted")
        %{state | status: :Idle, m_calibrated: false}
      _ -> state
    end
  end

  def m_calibrate(_command, state) do
    state
  end

  @doc """
  handles compensation message only when status is idle, otherwise ignore
  """
  def m_compensate(state)  when state.status == :Idle do
    Logger.info("CALIBRATION - mcompensate")
    # go_vertical(state)  # no pid is passed because message back must not be sent
    spawn(Engine.Calibration, :go_vertical, [state])  # no pid is passed because message back must not be sent
    pause(2)  # let AZ reading stabilize before going horizontal
    # starts the compensation data collection on the microcontroller side
    seconds = ceil((state.aim.pos_alt / @calispeedslow))
    spawn(Engine.Calibration, :go_horizontal, [state, self(), seconds])
    state
  end

  def m_compensate(status) do
    status
  end

  def untilt(_, state) do
    state
  end

  def a_calibrate(_, state) do
    state
  end

  # def m_calibrate(_command, state) do
  #   state
  # end

  # def a_calibrate_h(state) do
  #   state
  # end

  # def a_calibrate_v(state) do
  #   state
  # end

  # def a_calibration_save(state) do
  #   Circuits.UART.write(state.serial_pid, "export_a_calibration")  # starts the data collection on the microcontroller side
  #   state
  # end

  # Moves the tube to vertical position then stops and sends message back to Engine
  # these 3 functions can't be private because they are spawned directly
  # if pid is passed, a message is sent back to the pid, otherwise a message is not required.
  def go_vertical(state) do
    travel = 90 - state.aim.pos_alt
    Logger.info("Go vertical: rotate #{travel} starting from #{state.aim.pos_alt}")
    Motors.spin_deg({0, travel}, {0, @calispeedslow}, false)    # move scope to vertical - ramp = false
  end

  def go_vertical(state, pid) do
    travel = 90 - state.aim.pos_alt
    Logger.info("Go vertical: rotate #{travel} starting from #{state.aim.pos_alt}")
    Motors.spin_deg({0, travel}, {0, @calispeedslow}, false)    # move scope to vertical - ramp = false
    send(pid,{:calibration, "tube is vertical"})
  end

  def rotate_360(_state, pid) do
    Logger.info("Starting rotation 360°")
    Motors.spin_deg({360, 0}, {@calispeedfast, 0}, true)    # rotate scope 360° - ramp = true
    send(pid,{:calibration, "first step completed"})
  end

  def counterotate_360(_state, pid) do
    Logger.info("Starting rotation -360°")
    Motors.spin_deg({360, 0}, {-@calispeedfast, 0}, true)   # rotate scope -360° - ramp = true
    send(pid,{:calibration, "second step completed"})
  end

  def go_horizontal(state, pid, seconds) do
    dalt = 0 - state.aim.pos_alt
    Logger.debug("SENDING TO TEENSY: m_compensation #{seconds}")
    Circuits.UART.write(state.serial_pid, "m_compensation #{seconds}")  # starts the data collection on the microcontroller side
    Motors.spin_deg({0, dalt}, {0, -@calispeedslow}, true)    # move scope to horizontal - ramp = true
    # send(pid,{:calibration, "fourth step completed"})
    # send(pid,{:calibration, "third step completed"})
    send(pid,{:calibration, "compensation completed"})
  end

  defp sign(value) when value < 0 do
    -1
  end
  defp sign(value) do
    1
  end

  # needed to insert a pause between actions
  # in order to allow azimuth readings to stabilize
  defp pause(seconds) do
    :timer.sleep(1000*seconds)
  end

end
