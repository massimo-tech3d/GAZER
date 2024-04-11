defmodule Engine.Config do
  require Logger

  @config_file "/data/config.bin"

  defstruct timezone: "Europe/Rome",
            cal_speed: 3,
            stepper_conf: %{
              alt_worm: 144,  # worm ratio of AZ Mount
              az_worm:  144,  # worm ratio of AZ Mount
              alt_bp:    60,  # big_pulley - teeth of lerger pulley
              az_bp:     60,  # big_pulley - teeth of lerger pulley
              alt_sp:    20,  # small_pulley - teeth of smaller pulley
              az_sp:     20,  # small_pulley - teeth of smaller pulley
              alt_mds:  0.9,  # motor_degrees_step - degrees per step for the motors (equal for AZ and ALT)
              az_mds:   0.9,  # motor_degrees_step - degrees per step for the motors (equal for AZ and ALT)
              # xx_reverse serve ad invertire la polarità del motore, senza dover invertire il cavo
              alt_r:      1,  # alt_motor_reverse - alt aumenta --> :cw  alt decresce --> :ccw
              az_r:       1,  # az_motor_reverse - az aumenta --> :cw  az decresce --> :ccw
              alt_fspd: 480,  # steppers full steps per degree
              az_fspd:  480   # steppers full steps per degree
            },
            a_cal: %{
              OX: 0,
              GX: 1,
              OZ: 0,
              GZ: 1
            }

  # refactor
  # serve funzione per inizializzare il file nel caso non esista.
  # save_config deve poter salvare la configurazione anche per singola key, mantenendo le altre invariate

  def get_config(key \\ :all) do
    conf = case File.read(@config_file) do
      {:ok, contents} -> :erlang.binary_to_term(contents)
      _ -> save_config(%Engine.Config{})  # if file read error initializes with default configuration
            %Engine.Config{}
    end
    inspect(conf) |> Logger.debug
    case key do
      :tz -> conf.timezone
      :steppers -> conf.stepper_conf
      :a_cal -> conf.a_cal
      _ -> conf
    end
  end

  def save_config(config) do
    sc = config.stepper_conf
    sc = %{sc | az_fspd: fspd_az(sc), alt_fspd: fspd_alt(sc)}
    File.write!(@config_file, :erlang.term_to_binary(%Engine.Config{timezone: config.timezone, stepper_conf: sc, a_cal: config.a_cal}))
    sc  # returns just the stepper part of configuration
  end

  # if az_fspd is in map, and not 0, returns it. Otherwise calculates it
  defp fspd_az(mappa = %{az_fspd: fspd}) when fspd != 0 and fspd != "" do
    fspd
  end
  defp fspd_az(mappa) do
    # steppers full steps per deg = worm * big_p / small_p / motors_deg_step
    {worm, _} = mappa.az_worm
    {bp, _} = mappa.az_bp
    {sp, _} = mappa.az_sp
    {mds, _} = mappa.az_mds
    worm * bp / sp / mds
  end

  # if alt_fspd is in map, and not 0, returns it. Otherwise calculates it
  defp fspd_alt(mappa = %{alt_fspd: fspd}) when fspd != 0 and fspd != "" do
    fspd
  end
  defp fspd_alt(mappa) do
    # steppers full steps per deg = worm * big_p / small_p / motors_deg_step
    {worm, _} = mappa.alt_worm
    {bp, _} = mappa.alt_bp
    {sp, _} = mappa.alt_sp
    {mds, _} = mappa.alt_mds
    worm * bp / sp / mds
  end
end
