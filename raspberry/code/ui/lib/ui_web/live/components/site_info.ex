defmodule UiWeb.Components.SiteInfo do
  use UiWeb, :live_component
  use Phoenix.HTML
  alias UiWeb.Components
  require Logger

  def render(assigns) do
    ~H"""
    <fieldset class="border-solid border border-red-800 rounded-md p-4 m-1"><legend class="text-sm  ml-2 px-2">Site</legend>
      <div>
        <p>Date: <strong><%= @day %></strong> Time Zone: <strong><%= @tz %></strong></p>
        <p>Date: <strong><%= @day %></strong></p>
        <p>Local time: <strong><%= @hour %></strong> UTC: <strong><%= @utc %></strong></p>
        <p>Observing from Lat: <strong><%= @site_lat %></strong> Long: <strong><%= @site_long %></strong></p>
        <p>Mag field decl: <span class="font-bold"><%= @mag_dec %></span><span class="text-xl">°</span> Intensity: <span class="font-bold"><%= @mag_field %><span class="text-sm"> nT</span></span></p>
      </div>
    </fieldset>
    """
  end

  # inserisce in socket gli assign necessari al valore di default iniziale
  def mount(socket) do
    socket = socket
              |> assign(site_lat: "N/A", site_long: "N/A", mag_dec: "N/A", mag_field: ["N/A"])
              |> assign(day: now_date(), hour: now_time(), tz: "Europe/Rome")
    {:ok, socket}
  end

  # riceve da LV l'update degli assign cambiati
  def update(assigns, socket) do
    if assigns[:site_lat] == "N/A" or assigns[:site_long] == "N/A" do
      GenServer.cast(Engine, {:gui, "req_gps"})
      # Logger.debug("GUI - requested GPS update to engine")
    end
    # Logger.info("SITEINFO: TIMEZONE #{Map.get(socket.assigns, :tz)}")
    socket = assign(socket, day: now_date(), hour: now_time(Map.get(socket.assigns, :tz)), utc: now_time())
    {:ok, assign(socket, assigns)}
  end

  def now_date() do
    d = NaiveDateTime.local_now()
    "#{d.day}/#{d.month}/#{d.year}"
  end

  def now_time() do
    n = NaiveDateTime.local_now()  # this is not local time, this is GMT
    "#{pad(n.hour, 2)}:#{pad(n.minute, 2)}:#{pad(n.second, 2)}"
  end

  def now_time(timezone) do
    {:ok, n} = DateTime.now(timezone, Tzdata.TimeZoneDatabase)  # uncomment when not standalone
    "#{pad(n.hour, 2)}:#{pad(n.minute, 2)}:#{pad(n.second, 2)}"
  end

  defp pad(i, n) do
    Integer.to_string(i) |> String.pad_leading(n, "0")
  end
end
