defmodule UiWeb.PageController do
  use UiWeb, :controller

  require Logger

  # FOR OFFLINE TESTING
  # def conf do
  #   %{
  #     timezone: "Rome",
  #     cal_speed: 3,  # deg per second during calibration
  #     stepper_conf: %{
  #       az_worm:  144,   # whorm ratio of AZ Mount
  #       alt_worm: 144,  # whorm ratio of AZ Mount
  #       az_bp:    60,     # big_pulley - teeth of lerger pulley
  #       alt_bp:   60,    # big_pulley - teeth of lerger pulley
  #       az_sp:    20,     # small_pulley - teeth of smaller pulley
  #       alt_sp:   20,    # small_pulley - teeth of smaller pulley
  #       az_mds:   0.9,    # motor_degrees_step - degrees per step for the motors (equal for AZ and ALT)
  #       alt_mds:  0.9,   # motor_degrees_step - degrees per step for the motors (equal for AZ and ALT)
  #       az_r:     1,  # reverted --> -1 -- not reverted --> 1
  #       alt_r:    1,  # reverted --> -1 -- not reverted --> 1
  #       az_fspd:  480,   # steppers full steps per degree
  #       alt_fspd: 480   # steppers full steps per degree
  #     }
  #   }
  # end


  def index(conn, _params) do
    render(conn, "index.html", :page_title, "GAZER")
  end

  def motor_speed(conn, _params) do
    speeds = %{"1" => 1, "1.5" => 1.5, "2" => 2, "2.5" => 2.5, "3" => 3}

    render(conn, :motor_speed, speeds: speeds, speed: 0)
  end

  def speed_test(conn, _parameters) do
    speed = conn.params["speed"]
    GenServer.cast(Engine, {:gui, {:speed_test, speed}})
    redirect(conn, to: "/motor_speed")
  end

  def stop_test(conn, _parameters) do
    GenServer.cast(Engine, {:gui, :stop_speed_test})

    redirect(conn, to: "/motor_speed")
  end

  def speed_test_end(conn, _parameters) do
    GenServer.cast(Engine, {:gui, :stop_speed_test})
    redirect(conn, to: "/")
  end

  def config(conn, _params) do
    conf = Engine.get_configuration(:all) |> Map.from_struct()
    stp = conf.stepper_conf
    stp = %{stp | az_r: if stp.az_r == 1 do false else true end, alt_r: if stp.alt_r == 1 do false else true end}
    conf = %{conf | stepper_conf: stp}

    # inspect(conf) |> Logger.debug()
    # Map.put(conf, :cal_speed, 3)
    # Logger.debug("again")
    # inspect(conf) |> Logger.debug()

    render(conn, :configuration, conf: conf)
  end

  def update_configuration(conn, _parameters) do
      conf = %{
        timezone: conn.params["tz"],
        cal_speed: conn.params["cal_speed"] |> number,  # deg per second during calibration
        stepper_conf: %{
          az_worm:  conn.params["az_worm"] |> number,   # whorm ratio of AZ Mount
          alt_worm: conn.params["alt_worm"] |> number,  # whorm ratio of AZ Mount
          az_bp:    conn.params["az_bp"] |> number,     # big_pulley - teeth of lerger pulley
          alt_bp:   conn.params["alt_bp"] |> number,    # big_pulley - teeth of lerger pulley
          az_sp:    conn.params["az_sp"] |> number,     # small_pulley - teeth of smaller pulley
          alt_sp:   conn.params["alt_sp"] |> number,    # small_pulley - teeth of smaller pulley
          az_mds:   conn.params["az_mds"] |> number,    # motor_degrees_step - degrees per step for the motors (equal for AZ and ALT)
          alt_mds:  conn.params["alt_mds"] |> number,   # motor_degrees_step - degrees per step for the motors (equal for AZ and ALT)
          az_r:     if conn.params["az_r"] == "true" do -1 else 1 end,  # reverted --> -1 -- not reverted --> 1
          alt_r:    if conn.params["alt_r"] == "true" do -1 else 1 end,  # reverted --> -1 -- not reverted --> 1
          az_fspd:  conn.params["az_fspd"] |> number,   # steppers full steps per degree
          alt_fspd: conn.params["alt_fspd"] |> number   # steppers full steps per degree
        }
      }
      GenServer.cast(Engine, {:gui, {:new_conf, conf}})
      redirect(conn, to: "/")
  end

  def factory_reset(conn, _parameters) do
      conf = %{
        timezone: "Europe/Rome",
        cal_speed: 3,
        stepper_conf: %{
          az_worm: 144,   # whorm ratio of AZ Mount
          alt_worm: 144,  # whorm ratio of AZ Mount
          az_bp:   60,    # big_pulley - teeth of lerger pulley
          alt_bp:   60,   # big_pulley - teeth of lerger pulley
          az_sp:   20,    # small_pulley - teeth of smaller pulley
          alt_sp:   20,   # small_pulley - teeth of smaller pulley
          az_mds:  0.9,   # motor_degrees_step - degrees per step for the motors (equal for AZ and ALT)
          alt_mds:  0.9,  # motor_degrees_step - degrees per step for the motors (equal for AZ and ALT)
          # xx_r(everse) serve ad invertire la polarità del motore, senza dover invertire il cavo
          az_r:  1,       # az_motor_reverse - az aumenta --> :cw  az decresce --> :ccw
          alt_r:  1,      # az_motor_reverse - az aumenta --> :cw  az decresce --> :ccw
          az_fspd: 480,   # steppers full steps per degree
          alt_fspd: 480   # steppers full steps per degree
        }
      }
      render(conn, :configuration, conf: conf)
    end

    defp number(n) do
      {f, _} = Float.parse(n)
      f
    end
end
