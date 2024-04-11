defmodule Engine.Motors do
  require Logger

  @moduledoc """
  Calcola il movimento necessario per portare le coordinate attuali a coincidere con il target.
    Se la distanza è lunga più di X gradi (puntamento) fa un movimento veloce, altrimenti lo fa più lento
    (VALUTARE SE SEONO SUFFICIENTI DUE VELOCITA' OPPURE SE NE SERVE UNA TERZA ANCORA PIÙ LENTA PER
    L'INSEGUIMENTO VERO E PROPRIO)
  Riceve anche i comandi per il movimento manuale. Questi hanno priorità su quelli automatici e servono
  per le correzioni, oltre che per lo star hopping manuale.

  Speed 2 ~ 0.22°  al secondo -- 90° in 6.6 minuti  (circa) - 13' al secondo (diametro lunare in 2 ~ 3 secondi)
  Speed 16 ~ 1.8° al secondo -- 90° in 50"  (circa)
  """
  @stop     0
  @slew     0.25  # 0.25 gradi/sec
  @slewX2   0.5   # 0.5 gradi/sec
  @slewX4   1     # 1 grado/sec
  @slewX8   2     # 2 gradi/sec
  @slewX12  3     # 3 gradi/sec

  @doc """
  Stops motor movements.
  If was a manual correction, the motors will be restarted soon by Engine
  """
  def stop_motors() do
    Steppers.stop_all()
  end

  def spin_deg(deltas, speeds, ramp\\false) do
    # Logger.debug("****** DELTAS ******")
    # inspect(deltas) |> Logger.debug
    Steppers.spin_degrees(deltas, speeds, ramp)
  end

  # accepts a MAP. Keys could be either :speed_az or :speed_alt or both
  # def move_motors(motors, pid) do
  def move_motors(motors, pid) do
    Steppers.spin(motors)
    # gyro_x = 0
    # gyro_y = Map.get(motors, :speed_alt, 0)
    # gyro_z = Map.get(motors, :speed_az, 0)
    # message_to_sensors("gyro_val #{gyro_x} #{gyro_y} #{gyro_z}", pid)
  end

  @doc """
    Receives the list (up to two, like the J axes) of J directions
    either can be Strong or Mild.
    Activates the motors according to the requested axes, speed and direction

    TODO: ensure that Stepper.spin stops the motor that is not included in this list. In case it was activated by the previous command
    non va bene. Se mando i due comandi in sequenza, il secondo mi fermerebbe l'altro motore

    Devo comporli in una sola chiamata a Stepper.spin. Così posso far fermare il motore mancante. Modificare la manual(data) separando
      la creazione della/e map del/i motore/i e la chiamata a Stepper.spin

    mode can be :fast or :slow.
      :fast is used to freely move the scope and search for object
      :slow is used to guide and manually correct tracking -- much slower movements

  """
  def manual(data, mode, pid) do
    # Enum.map(data, fn command -> motore(command) |> Steppers.spin end)
    mot = Enum.map(data, fn command -> motore(command, mode) end)
    if length(mot) == 2 do
      Map.merge(List.first(mot), List.last(mot)) |> move_motors(pid)
    else
      List.first(mot) |> move_motors(pid)
    end
  end

  defp motore("LS", mode) do
    Logger.debug("MOTORE LS")
    # move = %{speed_az: -@slewX8}
    case mode do
      :fast -> %{speed_az: -@slewX8}
      :slow -> %{speed_az: -@slewX2}
    end
  end

  defp motore("LM", mode) do
    Logger.debug("MOTORE LM")
    case mode do
      :fast -> %{speed_az: -@slewX2}
      :slow -> %{speed_az: -@slew}
    end
  end

  defp motore("RS", mode) do
    Logger.debug("MOTORE RS")
    case mode do
      :fast -> %{speed_az: @slewX8}
      :slow -> %{speed_az: @slewX2}
    end
  end

  defp motore("RM", mode) do
    Logger.debug("MOTORE RM")
    case mode do
      :fast -> %{speed_az: @slewX2}
      :slow -> %{speed_az: @slew}
    end
  end

  defp motore("TS", mode) do
    Logger.debug("MOTORE TS")
    case mode do
      :fast -> %{speed_alt: @slewX8}
      :slow -> %{speed_alt: @slewX2}
    end
  end

  defp motore("TM", mode) do
    Logger.debug("MOTORE TM")
    case mode do
      :fast -> %{speed_alt: @slewX2}
      :slow -> %{speed_alt: @slew}
    end
  end

  defp motore("BS", mode) do
    Logger.debug("MOTORE BS")
    case mode do
      :fast -> %{speed_alt: -@slewX8}
      :slow -> %{speed_alt: -@slewX2}
    end
  end

  defp motore("BM", mode) do
    Logger.debug("MOTORE BM")
    case mode do
      :fast -> %{speed_alt: -@slewX2}
      :slow -> %{speed_alt: -@slew}
    end
  end

  # Illegal command. Just stops the motor
  defp motore(command, _mode) do
    Logger.debug("MOTORE ILLEGAL --#{command}--")
    move = %{speed_alt: @stop}
  end

  defp message_to_sensors(text, pid) do
    Circuits.UART.write(pid, "#{text}")
  end
end
