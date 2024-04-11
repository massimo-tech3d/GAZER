defmodule Engine.Goto do
  @moduledoc """
  responds to the external events when in goto mode. Events can be:
  - Sensors reading
  - Calibration
  - Joystick move
  - Joystick button
  - GUI request

    From :Goto, the state can change to :Tracking (closeness) or :Idle (button event)
  """

  require Logger

  @stop     0
  @slew     0.25  # 0.25 gradi/sec
  @slewX2   0.5   # 0.5 gradi/sec
  @slewX4   1     # 1 grado/sec
  @slewX8   2     # 2 gradi/sec
  @slewX12  3     # 3 gradi/sec

  @tt       0.5   # tracking_thresh

  @doc """
    handles message from gui.
    :goto :dso
    :goto :planet
    anything else is ignored
  """
  def gui({:goto, :dso, obj}, state) do
    goto_dso(obj, %{state | status: :Goto})
  end

  def gui({:goto, :planet, planet}, state) do
    goto_planet(planet, %{state | status: :Goto})
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

  # def m_calibrate(_command, state) do
  #   state
  # end


  # def a_calibrate_h(state) do
  #   state
  # end

  # def a_calibrate_v(state) do
  #   state
  # end

  @doc """
  response to joystick event in goto mode only button is handled - aborts goto
  """
  def joystick(["button"], state) do
    target = %{defined: false, label: "", ar: 0, dec: 0}
    state = %{state | status: :Idle, manual: false, target: target}
  end

  def joystick(_data, state) do
    state
  end

  @doc """
  response to sensors event in goto mode
  - updates the state and updates the GUI via aim_change
  - if distance to object below threshold, changes status to :Tracking
  - returns the new state
  """
  def sensors(payload, state) do
    state = Engine.Aim.aim_change(payload, state)
    {da, dz} = delta_altaz(state)
    if abs(da) < @tt and abs(dz) < @tt do
      %{state | status: :Tracking}
    else
      go(%{da: da, dz: dz, lat: state.site_lat}, state.serial_pid)
      state
    end
  end

  def goto_dso(obj, state) do
    label = (obj[:id] |> String.upcase) <> if obj[:messier] != "" do " - M#{obj[:messier]}" else "" end
    ra = obj[:ar] |> Astrex.Common.hms2hours |> Astrex.Common.hours2deg
    dec = obj[:decl] |> Astrex.Common.dms2deg
    target = %{defined: true, label: label, ar: ra, dec: dec}
    %{state | target: target}
  end

  def goto_planet(planet, state) do
    # where_is returns ra in hours, dec in degrees
    coords = Astrex.where_is(String.to_atom(planet))
    target = %{defined: true, label: String.to_atom(planet), ar: coords.ra |> Astrex.Common.hours2deg, dec: coords.dec}
    %{state | target: target}
  end

  # returns the AZ and ALT gaps that needs to be filled to reach the target
  defp delta_altaz(state) do
    %{alt: gt_alt, az: gt_az} = Astrex.eq2az(%{ra: state.target.ar, dec: state.target.dec})

    # Logger.debug("GOTO deltas: alt #{gt_alt - state.aim.pos_alt}  az #{gt_az - state.aim.pos_az}")
    {gt_alt - state.aim.pos_alt, gt_az - state.aim.pos_az}
  end

  # riceve le coordinate ALT/AZ a cui il telescopio sta puntando ORA
  # La mappa ricevuta contiene anche la key rot: con l'angolo di rotazione
  defp go(deltas, pid) do
    deltas
    |> speeds
    |> Engine.Motors.move_motors(pid)
  end

  defp speeds(deltas) do
    delta_alt = deltas.da
    delta_az = deltas.dz

    motors =
      cond do
        delta_alt < -5                        -> %{speed_alt: -@slewX8}
        delta_alt >= -5 and delta_alt < -2    -> %{speed_alt: -@slewX2}
        delta_alt >= -2 and delta_alt < -@tt  -> %{speed_alt: -@slew}
        # delta_alt >= -2                       -> %{speed_alt: -@slew}

        delta_alt > 5                         -> %{speed_alt: @slewX8}
        delta_alt <= 5 and delta_alt > 2      -> %{speed_alt: @slewX2}
        delta_alt <= 2 and delta_alt > @tt    -> %{speed_alt: @slew}
        # delta_alt <= 2                        -> %{speed_alt: @slew}
        true                                  -> %{speed_alt: 0}  # this should never be called
      end
    |> Map.merge(
      cond do
        delta_az < -5                         -> %{speed_az: -@slewX8}
        delta_az >= -5 and delta_az < -2      -> %{speed_az: -@slewX2}
        delta_az >= -2 and delta_az < -@tt    -> %{speed_az: -@slew}
        # delta_az >= -2                        -> %{speed_az: -@slew}

        delta_az > 5                          -> %{speed_az: @slewX8}
        delta_az <= 5 and delta_az > 2        -> %{speed_az: @slewX2}
        delta_az <= 2 and delta_az > @tt      -> %{speed_az: @slew}
        # delta_az <= 2                         -> %{speed_az: @slew}
        true                                  -> %{speed_az: 0}  # this should never be called
      end)
    motors
  end

end
