defmodule Engine.Idle do
  @moduledoc """
  responds to the external events when in idle mode. Events can be:
  - Sensors reading
  - Calibration
  - Joystick move
  - Joystick button
  - GPS info
  - GUI request

  From :Idle, the state can change to :Calibration (gui event) :Goto (gui event) or :Tracking (button event)
  """
  #require Logger
  alias Engine.Aim
  alias Engine.Motors
  alias Phoenix.PubSub

  @doc """
    handles message from gui.
    :goto :dso
    :goto :planet
    anything else is ignored
  """
  # def gui({:goto, :dso, cat, id}, state) do
  def gui({:goto, :dso, obj}, state) do
    Engine.Goto.goto_dso(obj, %{state | status: :Goto})
  end

  def gui({:goto, :planet, planet}, state) do
    Engine.Goto.goto_planet(planet, %{state | status: :Goto})
  end

  def gui(msg, state) do
    state
  end

  # def untilt(:hiz, state) do
  #   Circuits.UART.write(state.serial_pid, "untilt_hiz")  # starts the data collection on the microcontroller side
  #   state
  # end

  # def untilt(:hiz_r, state) do
  #   Circuits.UART.write(state.serial_pid, "untilt_hiz_r")  # starts the data collection on the microcontroller side
  #   state
  # end

  def untilt(:zero, state) do
    Circuits.UART.write(state.serial_pid, "untilt_zero")  # starts the data collection on the microcontroller side
    state
  end

  def m_calibrate(command, state) do
    Engine.Calibration.m_calibrate(command, state)  # starts the magnetometer calibration
  end

  def m_compensate(state) do
    Engine.Calibration.m_compensate(state)  # starts the magnetometer compensation
  end

  def a_calibrate(:hor, state) do
    Circuits.UART.write(state.serial_pid, "a_calibration_h")  # starts the data collection on the microcontroller side
    state
  end

  def a_calibrate(:vert, state) do
    Circuits.UART.write(state.serial_pid, "a_calibration_v")  # starts the data collection on the microcontroller side
    state
  end

  def a_calibrate(:save, state) do
    Circuits.UART.write(state.serial_pid, "export_a_calibration")  # requests to microcontroller the accel calibration data
    state
  end

  # def a_calibrate_h(state) do
  #   Circuits.UART.write(state.serial_pid, "a_calibration_h")  # starts the data collection on the microcontroller side
  #   state
  # end

  # def a_calibrate_v(state) do
  #   Circuits.UART.write(state.serial_pid, "a_calibration_v")  # starts the data collection on the microcontroller side
  #   state
  # end

  @doc """
  response to joystick event in idle mode
  - if joystick is centered stops the motor - no need to update the state, just returns it. Manual turned off
  - if button is pressed updates the GUI and changes the state.status to :tracking and returns it. Manual turned off
  - if joystick move, directs the motors accordingly and return the status as it is. Manual turned on
  """
  def joystick(["center"], state) do
    Motors.stop_motors()
    %{state | manual: false}
  end

  def joystick(["button"], state) do
    # state = Engine.send_to_gui(state, %{status: :tracking, goto_ra: state.pos_ar, goto_dec: state.pos_dec})  # no need to send, aim does this
    %{state | target: %{ar: state.aim.pos_ar, dec: state.aim.pos_dec, defined: false, label: ""}, status: :Tracking, manual: false}
  end

  def joystick(data, state) do
    Motors.manual(data, :fast, state.serial_pid)
    %{state | manual: true}
  end

  @doc """
  response to sensors event in idle mode
  - updates the state and the GUI via aim_change
  - returns the new state
  """
  def sensors(payload, state) do
    Aim.aim_change(payload, state)
  end

end
