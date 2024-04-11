defmodule UiWeb.UiLive do
  require Logger
  use UiWeb, :live_view
  alias UiWeb.Components.SiteInfo
  alias UiWeb.Components.ScopeInfo
  alias UiWeb.Components.Calibration
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

  # handles m_calibration completed event
  def handle_info("m_calibration completed", socket) do
    Logger.info("GUI: received mag calibration completed")
    send_update(Calibration, id: "calibration", m_cal: :ok)
    {:noreply, socket |> assign(m_cal: :ok)}
  end

  # handles mount_level event
  def handle_info("mount_level " <> theta, socket) do
    {itheta, _} = Integer.parse(theta)
    Logger.info("GUI: received mount level #{itheta}")
    send_update(Calibration, id: "calibration", theta: itheta)
    {:noreply, socket |> assign(theta: itheta)}
  end

  # handles m_compensation completed event
  def handle_info("m_compensation completed", socket) do
    Logger.info("GUI: received mag compensation completed")
    send_update(Calibration, id: "calibration", m_comp: :ok)
    {:noreply, socket |> assign(m_comp: :ok)}
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

  # Handles scope aim updates from Engine - NEWW
  def handle_info(%{status: status, aim: aim, target: target, target_az: taz, target_alt: talt}, socket) do
    send_update(ScopeInfo, id: "scope", tracking_status: status, target: target, target_alt: talt, target_az: taz, aim: aim)

    {:noreply, socket |> assign(tracking_status: status, aim: aim, target: target, target_az: taz, target_alt: talt)}
  end

  # goto or tracking terminated - interrupted by button - message from Engine
  def handle_info(%{status: :idle}, socket) do
    send_update(ScopeInfo, id: "scope", tracking_status: "IDLE", target: "", target_r_asc: "N/A", target_declination: "N/A")
    {:noreply, socket |> assign(tracking_status: "IDLE", target: "", target_r_asc: "N/A", target_declination: "N/A")}
  end

  # COMPONENT HANDLERS

  # handles calibration button press event
  # def handle_event("calibration_help", _value, socket) do
  #   Logger.debug("UI - help")
  #   {:noreply, socket}
  # end

  def handle_event("untilt_zero", _value, socket) do
    Logger.debug("UI - handle zero untilt")
    GenServer.cast(Engine, {:gui, "untilt_zero"})
    send_update(Calibration, id: "calibration", u_zero: :ok)
    {:noreply, socket |> assign(u_zero: :ok)}
  end

  # def handle_event("untilt_hiz_r", _value, socket) do
  #   Logger.debug("UI - handle untilt reversed")
  #   GenServer.cast(Engine, {:gui, "untilt_hiz_r"})
  #   send_update(Calibration, id: "calibration", u_hiz_r: :ok)
  #   {:noreply, socket |> assign(u_hiz_r: :ok)}
  # end

  def handle_event("sensor_block_horizontal", _value, socket) do
    Logger.debug("UI - handle sensor_block_horizontal")
    GenServer.cast(Engine, {:gui, "sensor_block_horizontal"})
    send_update(Calibration, id: "calibration", block_h: :ok) # potrei usare questo update per settare il colore del bottone selezionato
    send_update(Calibration, id: "calibration", block_v: :nok) # potrei usare questo update per settare il colore del bottone selezionato
    {:noreply, socket |> assign(u_zero: :ok)}
  end

  def handle_event("sensor_block_vertical", _value, socket) do
    Logger.debug("UI - handle sensor_block_vertical")
    GenServer.cast(Engine, {:gui, "sensor_block_vertical"})
    send_update(Calibration, id: "calibration", block_h: :nok) # potrei usare questo update per settare il colore del bottone selezionato
    send_update(Calibration, id: "calibration", block_v: :ok) # potrei usare questo update per settare il colore del bottone selezionato
    {:noreply, socket |> assign(u_zero: :ok)}
  end

  def handle_event("m_calibration", _value, socket) do
    Logger.debug("UI - handle event m_calibration")
    GenServer.cast(Engine, {:gui, "m_calibration"})
    send_update(Calibration, id: "calibration", m_cal: :progress)
    {:noreply, socket |> assign(m_cal: :progress)}
  end

  def handle_event("m_compensation", _value, socket) do
    Logger.debug("UI - handle event m_compensation")
    GenServer.cast(Engine, {:gui, "m_compensation"})
    send_update(Calibration, id: "calibration", m_comp: :progress)
    {:noreply, socket |> assign(m_comp: :progress)}
  end

  def handle_event("a_calibration_h", _value, socket) do
    Logger.debug("UI - handle event a_calibration_h")
    GenServer.cast(Engine, {:gui, "a_calibration_h"})
    send_update(Calibration, id: "calibration", a_cal_h: :ok)
    {:noreply, socket |> assign(a_cal_h: :ok)}
  end

  def handle_event("a_calibration_v", _value, socket) do
    Logger.debug("UI - handle event a_calibration_v")
    GenServer.cast(Engine, {:gui, "a_calibration_v"})
    send_update(Calibration, id: "calibration", a_cal_v: :ok)
    {:noreply, socket |> assign(a_cal_v: :ok)}
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
      acc = Keyword.merge(acc, ["#{id} | const: #{const} | mag: #{mag} #{mess}": id])
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
    [h, m, s] = String.split(ra, ":")
    [s | _] = String.split(s, ".")
    "#{h}h #{m}' #{s}\""
  end

  defp format_ra(ra) do
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

  def degree_hour_format("N/A") do
    "N/A"
  end

  def degree_hour_format(h) do
    normal =24*h/360
    hours = trunc(normal)
    m = abs(normal - trunc(normal))
    min = trunc(m*60)
    sec = trunc((m*60 - trunc(m*60))*60)

    "#{hours}h #{min}' #{sec}\""
  end

  def angle_format("N/A") do
    "N/A"
  end

  def angle_format(angle) do
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
