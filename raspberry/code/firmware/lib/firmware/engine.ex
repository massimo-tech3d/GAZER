defmodule Engine do
  @moduledoc """
    The main coordinator of all possible inputs and followups:
      Joystick commands
      Sensors
      GUI commands
      GPS

    The GenServer maintains local coordinates, current ALT/AZ coordinates and AR/DEC to be tracked
    Basing on received inputs (sensors, joystick, gui) calls the motors handlers

    Keeps the GUI updated via the Phoenix.pubsub

    Message sources received as info:
      - Calibration (internal message from spawned process)
      - Joystick
      - Sensors

    Message sources received as casts:
      - GPS
      - Gui

    The Engine can be in the following states and reacts to the messages accordingly (and possiblt differently)
    States:
      - Calibration  - ignores all messages except calibration abort (TBD)
      - Goto
      - Idle
      - Tracking
  """
  use GenServer

  require Logger
  require Useful
  alias Phoenix.PubSub

  @gui_interval 200  # 100 milliseconds between gui messages

  defstruct site_lat: 0,        # latitude of observing site
            site_long: 0,       # longitude of observing site
            mag_dec: 0,         # magnetic declination - calculated per lat/long
            mag_field: 0,       # magnetic field intensity - calculated per lat/long

            aim: %{pos_az: 0,          # sensors readings in degrees
                   pos_alt: 0,         # sensors readings in degrees
                   pos_ar: 0,          # sensors readings converted to AR by Aim.sensor_message (mag decl correction applied) in degrees
                   pos_dec: 0,         # sensors readings converted to DEC by Aim.sensor_message in degrees
            },

            target: %{
                  defined: false,
                  label: "",          # target label
                  ar: 0,       # right ascension target of goto in degrees - stays fixed
                  dec: 0,      # declination target of goto in degrees - stays fixed
            },

            serial_pid: 0,      # uart genserver pid to message back to sensors
            tts: 0,             # tracking timestamp
            ttg: 0,             # gui sending timestamp
            sid_alt_speed: 0,   # current sidereal speed, when tracking in progress
            sid_az_speed: 0,    # current sidereal speed, when tracking in progress
            manual: false,      # holds the status of manual correction in progress
            status: :Idle,      # other states :Goto :Tracking :Calibration
            m_calibrated: false # have the magnetometer been calibrated ?

  # Usare per le coordinate SOLO ed ESCLUSIVAMENTE GRADI non DMS, HMS oppure hours. SOLO GRADI
  # Le conversioni da gradi a quel che serve le si fa solo quando servono (praticamente solo in GUI)
  # e attenzione che ASTREX restittuisce a volte ore, a volte gradi a volte hms/dms
  # CONVERTIRE PRIMA DI SALVARE IN STATE
  #
  # INVIA UN SOLO MESSAGGIO A GUI CON TUTTE QUELLE 5 INFORMAZIONI SULLO STATO
  # SE NON STAI INSEGUENDO, SETTA TARGET UGUALE A Idle
  # SE STAI INSEGUENDO SETTALO A STATUS (goto o tracking) + LABEL
  # COSI' DA GUI PRENDI TUTTO CIECAMENTE E LO TRATTI IN MODO UNIFORME.

  # client API

  def start_link(_state, _opts \\ []) do
    GenServer.start_link(__MODULE__, :ok, [name: __MODULE__])
  end

  # server API

  @doc """
    launches and initializes all necessary genservers:
    - Circuits.UART
    - Steppers
    - GPS reader
    - Astrex
  """
  def init(:ok) do
    {:ok, pid} = Circuits.UART.start_link
    status = Circuits.UART.open(pid, "ttyAMA0", speed: 115200, active: true)
    case status do
      :ok -> Circuits.UART.configure(pid, framing: {Circuits.UART.Framing.Line, separator: "\n"})
      {:error, what} -> Logger.error("UART ERROR #{what}")
    end

    # tz = Engine.Config.get_config(:tz)
    Steppers.init_motors(Engine.Config.get_config(:steppers))
    Gps.start_link()
    # if Astrex is launched by the Ui Application
    # Astrex.Server.start_link()    # initialized with default coordinates (Greenwich) until GPD is read
    upload_accel_calibration(pid)
    Logger.info("Launched Engine GenServer")
    {:ok, %Engine{serial_pid: pid}}
  end

  def handle_call(:ok, _from, state) do
    {:reply, state, state}
  end

  @doc """
  Receives cast from other processes:
    - GPS: The message contains Latitude, Longitude and current UTC from phone GPS, forwarded from Android app
           Sets the system time
           Updates the Astrex server
           Calculates the geomagnetic declination
           Updates the GUI immediately
           Saves all info in the state
    - GUI: Possible messages (except where stated all are forwarded to status modules and executed only in idle mode):
           - acquire the hard iron z axis for the untilter
           - calibrate magnetometer
           - compensate magnetometer
           - h and v calibration of accelerometer
           - save accel calibration
           - req_gps (handled here)
           - save general configuration (handled here)
           - motor speed test
           - goto object - dso/planet
  """
  def handle_cast({:gps, msg}, state) do
    utc = msg.utc
    System.cmd("date", ["-s", NaiveDateTime.to_string(utc)])
    Astrex.Server.set_ll(%{lat: msg.lat, long: msg.lon})
    {mag_dec, _dip, mag_field, _gv} = Astrex.mag_declination()
    send_to_gui(state, %{site_lat: msg.lat, site_long: msg.lon, mag_dec: mag_dec, mag_field: mag_field, tz: Engine.Config.get_config(:tz)})
    {:noreply, %{state | site_lat: msg.lat, site_long: msg.lon, mag_dec: mag_dec, mag_field: mag_field}}
  end

  def handle_cast({:gui, msg}, state) do
    state =
      case msg do
        # "untilt_hiz"         -> apply(:"Elixir.Engine.#{state.status}", :untilt, [:hiz, state])
        # "untilt_hiz_r"       -> apply(:"Elixir.Engine.#{state.status}", :untilt, [:hiz_r, state])
        "untilt_zero"             -> apply(:"Elixir.Engine.#{state.status}", :untilt, [:zero, state])
        "sensor_block_horizontal" -> Circuits.UART.write(state.serial_pid, "block_h")
        "sensor_block_vertical"   -> Circuits.UART.write(state.serial_pid, "block_v")
        "m_calibration"           -> Logger.debug("ENGINE m_calibration")
                                     apply(:"Elixir.Engine.#{state.status}", :m_calibrate, [:start, state])
        "m_compensation"          -> apply(:"Elixir.Engine.#{state.status}", :m_compensate, [state])
        "a_calibration_h"         -> apply(:"Elixir.Engine.#{state.status}", :a_calibrate, [:hor, state])
        "a_calibration_v"         -> apply(:"Elixir.Engine.#{state.status}", :a_calibrate, [:vert, state])
        "a_calibration_save"      -> apply(:"Elixir.Engine.#{state.status}", :a_calibrate, [:save, state])
        # "a_calibration_save" -> apply(:"Elixir.Engine.#{state.status}", :a_calibrate_save, [state])
        # "a_calibration_save" -> Elixir.Engine.Calibration.a_calibration_save(state)
        "req_gps"   -> send_to_gui(state, %{site_lat: state.site_lat, site_long: state.site_long, mag_dec: state.mag_dec, mag_field: state.mag_field, tz: Engine.Config.get_config(:tz)})
        {:new_conf, conf}    -> save_configuration(state, conf)

        {:speed_test, speed} -> test_motors(state, speed)
        :stop_speed_test     -> test_motors(state, 0)

        _                    -> apply(:"Elixir.Engine.#{state.status}", :gui, [msg, state])  # any goto request goes to the status modules
      end
    {:noreply, state}
  end

  def handle_cast(_, state) do
    {:noreply, state}
  end

  @doc """
    :calibration - implements the workflow for calibrating the magnetometer. three steps:
        0 puts tube vertical
        1 rotates 360°
        2 rotates -360°
        3 rotates to south maintaining alt 90°
        4 rotates to alt 0° maintaining az to south (180°)
    :circuits_uart
        handles UART messages (from arduino/teensy/ESM32). Messages can start with:
          SENSORS: changed ALT/AZ coordinates to which the telescope aims
          JOYSTICK: manual commands, corrections etc
          A_CAL: saves in configuration file the current accelerometer calibration, read from the teensy board

    All messages are comma separated
  """
  def handle_info({:calibration, msg}, state) do
    state =
      case msg do
        "tube is vertical"      -> apply(:"Elixir.Engine.#{state.status}", :m_calibrate, [:first_step, state])
        "first step completed"  -> apply(:"Elixir.Engine.#{state.status}", :m_calibrate, [:second_step, state])
        "second step completed" -> state
        "compensation completed" -> state
      end
    {:noreply, state}
  end

  def handle_info({:circuits_uart, "ttyAMA0", msg}, state) do
    case parse_uart(msg) do
      # applies the function corresponding to the message (sensors, calibration etc)
      # from the module corresponding to the status (idle, calibration, tracking etc)
      # TODO handle teensy messages like "Acc cal horizontal reading acquired"
      {"SENSORS", payload}  -> {:noreply, apply(:"Elixir.Engine.#{state.status}", :sensors, [payload, state])}
      {"JOYSTICK", payload} -> {:noreply, apply(:"Elixir.Engine.#{state.status}", :joystick, [payload, state])}
      {"A_CAL", payload}    -> {:noreply, save_accel_calibration(payload, state)}
      # {"M_CAL", ["points collected"]}    -> {:noreply, Engine.Calibration.m_calibrate(:third_step, state)}
      {"M_CAL", ["points collected"]}    -> {:noreply, send_to_gui(%{state | status: :Idle, m_calibrated: true}, "m_calibration completed")}
      {"MOUNT_LEVEL", theta}    ->  Logger.debug("Received THETA #{theta}")
                                    {:noreply, send_to_gui(state, "mount_level #{theta}")}
      {"M_COMP", ["compensation map completed"]}    -> {:noreply, send_to_gui(%{state | status: :Idle, m_calibrated: true}, "m_compensation completed")}
      _ -> Logger.info("Received unknown message from UART: .#{msg}.")
           {:noreply, state}
    end
  end

  @doc """
  Sends a message to the GUI
  """
  def send_to_gui(state, msg) do
    PubSub.broadcast(Ui.PubSub, "engine", msg)
    state
  end

  @doc """
  Conditionally sends a message to the GUI if a timeout has passed. Allows to filter sending too
  many messages when only the last one is significant. If it sends, saves new timeout in state
  """
  def send_to_gui_maybe(state, msg) when is_list(msg) do
    ttg = :os.system_time(:millisecond)
    if (ttg - state.ttg) >= @gui_interval do
      for mex <- msg, do: send_to_gui(state, mex)
      %{state | ttg: :os.system_time(:millisecond)}
    else
      state
    end
  end

  def send_to_gui_maybe(state, msg) do
    ttg = :os.system_time(:millisecond)
    if (ttg - state.ttg) >= @gui_interval do
      send_to_gui(state, msg)
      %{state | ttg: :os.system_time(:millisecond)}
    else
      state
    end
  end

  @doc """
  Reads the configuration and returns the requested key
  - :tz        -- timezone
  - :steppers  -- steppers configuration
  - :a_cal     -- accelerometer calibration
  - no key specified  -- full configuration
  """
  def get_configuration(key) do
    Engine.Config.get_config(key)
  end

  def save_configuration(state, conf) do
    Engine.Config.save_config(conf)
    |> Steppers.configuration_change()
    send_to_gui(state, %{timezone: conf.timezone})
    %{state | tz: conf.timezone}
  end

  ############### Private functions

  defp save_accel_calibration(calib, state) do
    [ox, gx, oz, gz] = calib
    Engine.Config.save_config(%{get_configuration(:all) | a_cal: %{OX: ox, GX: gx, OZ: oz, GZ: gz}})
    state
  end

  defp upload_accel_calibration(pid) do
    ac = Engine.Config.get_config(:a_cal)
    msg = "set_a_calibration,#{ac[:OX]},#{ac[:GX]},#{ac[:OZ]},#{ac[:GX]}"
    Circuits.UART.write(pid, msg)  # starts the data collection on the microcontroller side
  end

  # note: msg is returned as a list of strings
  defp parse_uart(s) do
    [code | msg] = String.replace(s, "\r", "") |> String.split(", ")
    if code == "JOYSTICK" do
      Logger.debug("ENGINE SENSOR MESSAGE - code #{code} msg #{msg}")
    end
    {code, msg}
  end

  defp test_motors(state, 0) do
    Logger.info("testing speed stopped")
    Steppers.stop_all()
    state
  end

  defp test_motors(state, speed) do
    {s, _} = Float.parse(speed)
    Logger.info("testing speed #{speed}")
    Steppers.spin(%{speed_az: s, speed_alt: 0})
    state
  end

end
