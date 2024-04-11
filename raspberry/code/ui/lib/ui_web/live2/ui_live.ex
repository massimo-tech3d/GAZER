defmodule UiWeb.UiLive2 do
  require Logger
  use UiWeb, :live_view
  alias UiWeb.Components.SiteInfo
  alias UiWeb.Components.ScopeInfo
  alias UiWeb.Components.PlanetSelection
  alias UiWeb.Components.DeepSkySelect
  alias UiWeb.Components.DeepSkySearch

  @moduledoc """
    The UI. Resides in a Liveview and feeds a number of livecomponents:
    * SiteInfo, which shows the site related information
      - date/time/TZ
      - location coordinates
      - magnetic declination
      The information are received from Engine when the satellite has been acquired
    * ScopeInfo, which shows the scope related information
      - aim direction Alt/Az
      - aim direction RA/Dec
      - GOTO/Tracking target, if any
      - Magnetometer Calibration button
      The information are received from Engine at every sensor update, not more often than 5 times/sec
    * PlanetSelection
      - allows to select a planet (or the Moon) to go to
      - checks the planet visibility and sends the name to Engine to go to.
    * DeepSkySearch
      - allows to select a DSO to go to according to search parameters
      - retrieves the object and sends it to Engine to go to.
      - Only visible objects can be selected
    * DeepSkySelection
      - allows to choose a DSO by specifing its catalog and id
      - retrieves the object, checks visibility, and sends it to Engine to go to.

    TODO
    - Allow to choose the minimum altitude - now fixed to 15°
    - Allow to choose DSO with a maximum distance from current aim. So to see only objects close
      to curent aim and minimize wait time when slewing
      or simply sort the objects list basing on distance
    - Add version number of Ui, Firmware and Arduino code
  """


  @min_alt 15

  # mounts the LiveView
  # sends to self a ticker event every second, to enable current time update
  def mount(_params, _session, socket) do
    if connected?(socket), do: :timer.send_interval(1000, self(), :tick)
    socket = socket |> assign(page_title: "GAZER", site_lat: "N/A", site_long: "N/A", mag_dec: "N/A", mag_field: "N/A", tz: "Europe/Rome")

    {:ok, socket}
  end

  def update(assigns, socket) do
    {:ok, assign(socket, assigns)}
  end

  def render(assigns) do
    Phoenix.View.render(UiWeb.PageView, "index.html", assigns)
  end

  def handle_params(params, uri, socket) do
    # Invoked after mount and whenever there is a live patch event.
    UiWeb.Endpoint.subscribe("engine")
    {:noreply, socket}
  end

  def handle_call(msg, {}, socket) do
    # Invoked to handle calls from other Elixir processes.
  end

  def handle_cast(msg, socket) do
    # Invoked to handle casts from other Elixir processes.
  end

  # handles calibration aborted event - not implemented yet
  def handle_info("calibration aborted", socket) do
    Logger.info("GUI: received calibration aborted")
    {:noreply, socket}
  end

  # handles calibration completed event
  def handle_info("calibration completed", socket) do
    Logger.info("GUI: received calibration completed")
    {:noreply, socket}
  end

  # handles calibration progress events - not implemented yet
  def handle_info(%{calib: text, pos: p}, socket) do
    {:noreply, socket |> assign(tracking_status: "CAL ACCEL POS #{p}")}
  end

  # handles calibration progress events - not implemented yet
  def handle_info(%{calib: text}, socket) do
    case text do
      "Gyro calibrated" -> {:noreply, socket |> assign(tracking_status: "IDLE")}
      "Accel calibrated" -> {:noreply, socket |> assign(tracking_status: "IDLE")}
      "Calibration aborted" -> Logger.debug("Received ABORT message")
        {:noreply, socket |> assign(tracking_status: "IDLE")}
      _ -> {:noreply, socket}
    end
  end

  # Handles site information updates from Engine
  def handle_info(%{site_lat: lat, site_long: long, mag_dec: mag_dec, mag_field: mag_field, tz: tz}, socket) do
    send_update(SiteInfo, id: "site", site_lat: two_decim(lat), site_long: two_decim(long), mag_dec: two_decim(mag_dec), mag_field: two_decim(mag_field), tz: tz)
    {:noreply, socket |> assign(site_lat: two_decim(lat), site_long: two_decim(long), mag_dec: two_decim(mag_dec), mag_field: two_decim(mag_field), tz: tz)}
  end

  def handle_info(%{timezone: tz}, socket) do
    Logger.info("UILIVE #{tz}")
    send_update(SiteInfo, tz: tz)
    {:noreply, socket |> assign(tz: tz)}
  end

  # Handles scope aim updates from Engine
  def handle_info(%{alt: alt, az: az, ra: ra, dec: dec}, socket) do
    send_update(ScopeInfo, id: "scope", altitude: angle_format(alt), azimuth: angle_format(az),
                r_asc: degree_hour_format(ra), declination: angle_format(dec))
    {:noreply, socket |> assign(altitude: alt, azimuth: az, r_asc: degree_hour_format(ra), declination: angle_format(dec))}
  end

  # Handles scope aim updates from Engine
  def handle_info(%{alt: alt, az: az, ra: ra, dec: dec, taz: taz, talt: talt}, socket) do
    send_update(ScopeInfo, id: "scope", altitude: angle_format(alt), azimuth: angle_format(az),
                r_asc: degree_hour_format(ra), declination: angle_format(dec), taz: angle_format(taz), talt: angle_format(talt))
    {:noreply, socket |> assign(altitude: alt, azimuth: az, r_asc: degree_hour_format(ra), declination: angle_format(dec))}
  end

  # Handles scope GOTO updates from Engine
  def handle_info(%{status: :goto, goto_ra: ra, goto_dec: dec, target: target}, socket) do
    ra = ra |> Astrex.Common.hours2hms |> format_ra()
    dec = dec |> Astrex.Common.deg2dms |> format_dec()
    send_update(ScopeInfo, id: "scope", tracking_status: "Slewing to", target: target, target_r_asc: ra, target_declination: dec)
    {:noreply, socket |> assign(tracking_status: "Slewing to", target: target, target_r_asc: ra, target_declination: dec)}
  end
  # Handles scope entering in TRACKING mode message from Engine
  def handle_info(%{status: :tracking, goto_ra: ra, goto_dec: dec}, socket) do
    ra = ra |> Astrex.Common.hours2hms |> format_ra()
    dec = dec |> Astrex.Common.deg2dms |> format_dec()
    send_update(ScopeInfo, id: "scope", tracking_status: "Tracking", target_r_asc: ra, target_declination: dec)
    {:noreply, socket |> assign(tracking_status: "Tracking", target_r_asc: ra, target_declination: dec)}
  end
  # goto or tracking terminated - interrupted by button - message from Engine
  def handle_info(%{status: :idle}, socket) do
    send_update(ScopeInfo, id: "scope", tracking_status: "IDLE", target: "", target_r_asc: "N/A", target_declination: "N/A")
    {:noreply, socket |> assign(tracking_status: "IDLE", target: "", target_r_asc: "N/A", target_declination: "N/A")}
  end

  # COMPONENT HANDLERS

  # handles calibration button press event
  def handle_event("m_calibration", _value, socket) do
    GenServer.cast(Engine, {:gui, "m_calibrate"})
    # TODO change button to "abort calibration" - not implemented yet
    {:noreply, socket}
  end

  def handle_event("a_calibration_h", _value, socket) do
    GenServer.cast(Engine, {:gui, "a_calibration_h"})
    {:noreply, socket}
  end

  def handle_event("a_calibration_v", _value, socket) do
    GenServer.cast(Engine, {:gui, "a_calibration_v"})
    {:noreply, socket}
  end

  def handle_event("a_calibration_save", _value, socket) do
    GenServer.cast(Engine, {:gui, "a_calibration_save"})
    {:noreply, socket}
  end

  # handles DSO search queries to feed the list of visible DSOs for further selection
  def handle_info({:search_dso, selection = %{"dso_filter" => type, "messier_only" => mes, "max_mag" => magnitude}}, socket) do
    objects = Astrex.DeepSky.select_objects(String.to_integer(magnitude), String.to_atom(type), mes == "true", @min_alt)
    objects = Enum.reduce(objects, [], fn o, acc ->
      [id, kind, ar, decl, const, mag, mess] = o
      mess = if mess != "" do " | M#{mess}" else "" end
      acc = Keyword.merge(acc, ["#{id} | const: #{const} | magnitude: #{mag} #{mess}": id])
    end)
    send_update(DeepSkySearch, id: "search-dso", dso_type: type, messier_only: mes, max_mag: magnitude, objects: objects)
    Logger.debug("UI SEARCH DSO: #{inspect(socket |> assign(dso_type: type, messier_only: mes, max_mag: magnitude))}")
    {:noreply, socket |> assign(dso_type: type, messier_only: mes, max_mag: magnitude)}
  end

  # handles DSO selected - puts a warning flash if the object is not visible
  # otherwise, sends the object to Engine for go to
  def handle_info({:goto_dso, cat, id}, socket) do
    object = Astrex.DeepSky.find_object(cat, id, @min_alt)
    object |> Kernel.inspect(label: "OBJECT: ") |> Logger.debug()
    socket = if object == %{} do
      put_flash(socket, :error, "#{cat}#{id} is below the minimun altitude (#{@min_alt}°)")
    else
      label = "#{cat}#{id}"
      send_update(ScopeInfo, id: "scope", message: label)
      Logger.debug("UI CASTING ENGINE FOR DSO GOTO")
      GenServer.cast(Engine, {:gui, {:goto, :dso, object}})
      socket
    end
    Logger.debug("UI GOTO DSO: #{inspect(socket)}") # TODO ERROR HERE --> socket = :ok instead of a socket
    {:noreply, socket}
  end

  # handles Solar System object selected - puts a warning flash if the object is not visible
  # otherwise, sends the name (planet or moon) to Engine for go to
  def handle_info({:goto_planet, planet}, socket) do
    coords = Astrex.where_is(String.to_atom(planet))
    coords = %{ra: coords.ra |> Astrex.Common.hours2deg, dec: coords.dec}

    %{alt: alt, az: _az} = Astrex.eq2az(coords)
    socket = if alt < @min_alt do
      put_flash(socket, :error, "#{planet} is below the minimun altitude (#{@min_alt}°)")
    else
      Logger.debug("UI CASTING ENGINE FOR PLANET GOTO")
      GenServer.cast(Engine, {:gui, {:goto, :planet, planet}})
      send_update(ScopeInfo, id: "scope", message: planet)
      socket
    end
    Logger.debug("UI SEARCH PLANET")
    {:noreply, socket |> assign(selected_planet: planet |> String.capitalize())}
  end

  # handles the tick event to update the current time
  def handle_info(:tick, socket) do
    send_update(SiteInfo, id: "site", site_lat: socket.assigns[:site_lat], site_long: socket.assigns[:site_long], mag_dec: socket.assigns[:mag_dec], mag_field: socket.assigns[:mag_field])
    {:noreply, socket}
  end

  # catch all clause
  def handle_info(msg, socket) do
    Logger.error("GUI MESSAGE RECEIVED UNKNOWN MESSAGE")
    Kernel.inspect(msg) |> Logger.debug
    {:noreply, socket}
  end

  # uitility functions

  defp format_ra(ra) when is_bitstring(ra) do
    Logger.warning("ra is bitstring")
    [h, m, s] = String.split(ra, ":")
    [s | _] = String.split(s, ".")
    "#{h}h #{m}' #{s}\""
  end

  defp format_ra(ra) do
    Logger.warning("ra is NOT bitstring")
    ra = ra |> Astrex.Common.deg2hours |> Astrex.Common.hours2hms
    [h, m, s] = String.split(ra, ":")
    [s | _] = String.split(s, ".")
    "#{h}h #{m}' #{s}\""
  end

  defp format_dec(dec) when is_bitstring (dec) do
    [d, m, s] = String.split(dec, ":")
    [s | _] = String.split(s, ".")
    "#{d}° #{m}' #{s}\""
  end

  defp format_dec(dec) do
    dec = dec |> Astrex.Common.deg2dms
    [d, m, s] = String.split(dec, ":")
    [s | _] = String.split(s, ".")
    "#{d}° #{m}' #{s}\""
  end

  defp degree_hour_format(h) do
    normal =24*h/360
    hours = trunc(normal)
    m = abs(normal - trunc(normal))
    min = trunc(m*60)
    sec = trunc((m*60 - trunc(m*60))*60)

    "#{hours}h #{min}' #{sec}\""
  end

  defp angle_format(angle) do
    deg = trunc(angle)
    m = abs(angle - trunc(angle))
    min = trunc(m*60)
    sec = trunc((m*60 - trunc(m*60))*60)
    "#{deg}° #{min}' #{sec}\""
  end

  defp two_decim(number) do
    round(number * 100) / 100
  end
end
