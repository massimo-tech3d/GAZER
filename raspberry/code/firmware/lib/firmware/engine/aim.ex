defmodule Engine.Aim do
  @moduledoc """
    Handles an aim change event
  """
  require Logger
  alias Astrex
  # alias Phoenix.PubSub

  @doc """
    Reacts to a ALT/AZ aim change:
      calculates the conversions to RA/DEC and, if a GOTO action is in progress calls the motor functions

    Keeps the GUI updated via the Phoenix.pubsub

    Receives the Engine state struct, which holds all necessary information

    The message to the gui is a map including:
    - status
    - alt, az and ra, dec -> coordinates where the telescope is aiming
    - target_az, target_alt and target_ra, target_dec -> coordinates of the target, if a target is selected
  """
  def aim_change(msg, state) do
    case sensors_message(msg, state) do
      {:ok, sensors} ->
        aim = %{pos_az: sensors.az, pos_alt: sensors.alt, pos_ar: sensors.ra, pos_dec: sensors.dec}
        state = %{state | aim: aim}
        mex = %{status: state.status, aim: aim} |> add_target(state)
        Engine.send_to_gui_maybe(state, mex)  # returns state
      {:error, _reason} ->
        state
    end
  end

  @doc """
    Same as aim_change but shows fixed the equatorial coordinates.
    they would otherwise be quite noisy, when tracking, due to floating point operations
    while tracking is quite precise.

    TDB così è identica all'altra, devo sostituire nel mex (non nello state!!!) le coordinate target fisse ar/dec, per fare in modo che non oscillino...
  """
  def aim_change_fixed(msg, state) do
    case sensors_message(msg, state) do
      {:ok, sensors} ->
        aim = %{pos_az: sensors.az, pos_alt: sensors.alt, pos_ar: sensors.ra, pos_dec: sensors.dec}
        state = %{state | aim: aim}
        mex = %{status: state.status, aim: aim} |> add_target(state)
        Engine.send_to_gui_maybe(state, mex)
      {:error, _reason} ->
        state
    end
  end

  #   returns a map with the data coming from the sensors in a usable format
  #   includes the ALT/AZ coordinates converted to Equatorial
  #   all data expressed in degrees
  defp sensors_message(msg, state) do
    try do
      ["AZ", az, "ALT", alt] = msg
      {f_az, _} = Float.parse(String.trim(az))
      {f_alt, _} = Float.parse(String.trim(alt))
      # dovrebbe essere il solo punto dove aggiungere la correzione della magnetic declination
      f_az = f_az + state.mag_dec  # NOTA: dovrebbe essere questo, provo a cambiare il segno per vedere se l'errore AZ diminuisce
      # f_az = f_az - state.mag_dec
      res = Astrex.az2eq(%{alt: f_alt, az: f_az}) |> Map.merge(%{alt: f_alt, az: f_az})
      {:ok, res}
    rescue
      _ -> Logger.error("Engine.Aim.sensors_message - Error parsing message")
           {:error, "error"}
    end
  end

  defp add_target(mex, state) do
    if state.target.defined do
      %{alt: alt, az: az} = Astrex.eq2az(%{ra: state.target.ar, dec: state.target.dec})
      Map.merge(mex, %{target: state.target, target_az: az, target_alt: alt})
    else
      Map.merge(mex, %{target: state.target, target_az: "N/A", target_alt: "N/A"})
    end
  end
end
