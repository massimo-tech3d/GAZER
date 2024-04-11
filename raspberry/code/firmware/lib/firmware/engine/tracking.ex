defmodule Engine.Tracking do
  @moduledoc """
  responds to the external events when in tracking mode. Events can be:
  - Sensors reading
  - Joystick move
  - Joystick button
  - GUI request
  - calibration request

  From :Tracking, the state can change to :Idle (button event) or :Goto (gui event)
  """
  require Logger
  alias Engine.Motors
  alias Engine.Aim
  alias Phoenix.PubSub

  @doc """
    handles message from gui.
    :goto :dso
    :goto :planet
    anything else is ignored
  """
  def gui({:goto, :dso, obj}, state) do
    Engine.Goto.goto_dso(obj, %{state | status: :Goto})
  end

  def gui({:goto, :planet, planet}, state) do
    Engine.Goto.goto_planet(planet, %{state | status: :Goto})
  end

  def gui(msg, state) do
    state
  end

  def untilt(_, state) do
    state
  end

  def m_calibrate(_command, state) do
    state
  end

  def m_compensate(_command, state) do
    state
  end

  def a_calibrate(_, state) do
    state
  end

  # def a_calibrate_h(state) do
  #   state
  # end

  # def a_calibrate_v(state) do
  #   state
  # end

  @doc """
  response to joystick event in tracking mode
  - if joystick is centered restarts sidereal speed
  - if button is pressed exits tracking mode. updates the GUI and changes the state.status to :Idle and returns it
  - if joystick move apply motor speed for guiding
  """
  def joystick(["center"], state) do
    # az_speed sign has been fixed already in track/1
    # Steppers.spin(%{speed_alt: state.sid_alt_speed, speed_az: state.sid_az_speed})
    Motors.move_motors(%{speed_alt: state.sid_alt_speed, speed_az: state.sid_az_speed}, state.serial_pid)
    %{state | manual: false}
  end

  def joystick(["button"], state) do
    Logger.info("Stops tracking")
    target = %{defined: false, label: "", ar: 0, dec: 0}
    state = %{state | status: :Idle, manual: false, target: target}
    # no need to send anything, everything will be sent with the next aim / sensor message
    # Engine.send_to_gui(state, %{status: :idle} )
  end

  def joystick(data, state) do
    Motors.manual(data, :slow, state.serial_pid)
    %{state | manual: true}
  end

  @doc """
  response to sensors event in tracking mode
  - ignores the sensors readings
  - if the interval (~ 1 second) calculates the sidereal speed basing on last known scope position
    and changes motors speed. Updates the state with the future altaz (after a second ofsidereal speed)
    and the timestamp.
  - returns the new state
  """
  def sensors(payload, state) do
    tts = :os.system_time(:millisecond)
    if tts - state.tts < 1000 do
      Aim.aim_change_fixed(payload, state)
    else
      track(state)  # returns updated state
    end
  end

  defp track(state) when state.manual == true do
    state
  end

  defp track(state) do
    # formula_based - alt - az - latitude
    {sid_speed_alt, sid_speed_az} = Astrex.sidereal_speeds(%{alt: state.aim.pos_alt, az: state.aim.pos_az})
    Motors.move_motors(%{speed_alt: sid_speed_alt, speed_az: sid_speed_az}, state.serial_pid)
    # calculates next altaz coordinates after one second of sidereal speed and the timestamp
    %{state | aim: %{state.aim | pos_alt: state.aim.pos_alt + sid_speed_alt, pos_az: state.aim.pos_az - sid_speed_az}, sid_alt_speed: sid_speed_alt, sid_az_speed: -sid_speed_az, tts: :os.system_time(:millisecond)}
  end

end
