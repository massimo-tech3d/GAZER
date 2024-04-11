defmodule UiWeb.Components.ScopeInfo2 do
  use UiWeb, :live_component
  use Phoenix.HTML
  alias UiWeb.Components
  alias UiWeb.Components.Stateless
  require Logger

  def render(assigns) do
    ~H"""
    <fieldset class="flex flex-col border-solid border border-red-800 rounded-md basis-full p-1 mx-1"><legend class="text-base ml-2 px-2">Scope</legend>
      <div class="flex flex-col">
        <div class="grid grid-cols-2">
          <fieldset class="grow border-solid border border-red-800 rounded-md p-1"><legend class="text-sm ml-2 px-2">Aim EQ</legend>
            <p>R.A: <span class="text-base font-bold"><%= @r_asc %></span></p>
            <p>Dec: <span class="text-base font-bold"><%= @declination %></span></p>
          </fieldset>
          <fieldset class="grow border-solid border border-red-800 rounded-md p-1 ml-1"><legend class="text-sm ml-2 px-2">Aim AZ</legend>
            <p>Alt: <span class="text-base font-bold"><%= @altitude %></span></p>
            <p>Az: <span class="text-base font-bold"><%= @azimuth %></span></p>
          </fieldset>
        </div>
        <div class="grid grid-cols-2">
          <fieldset class="grow border-solid border border-red-800 rounded-md p-1"><legend class="text-sm ml-2 px-2"><%= @tracking_status %> <%= @target %></legend>
            <p>R.A: <span class="text-base font-bold"><%= @target_r_asc %></span></p>
            <p>Dec: <span class="text-base font-bold"><%= @target_declination %></span></p>
          </fieldset>
          <fieldset class="grow border-solid border border-red-800 rounded-md p-1 ml-1"><legend class="text-sm ml-2 px-2">Target AZ</legend>
              <p>Alt: <span class="text-base font-bold"><%= @target_alt %></span></p>
              <p>Az: <span class="text-base font-bold"><%= @target_az %></span></p>
          </fieldset>
        </div>
      </div>
      <div class="flex flex-row justify-center">
        <fieldset class="flex justify-center grow self-center border-solid border border-red-800 rounded-md p-1 my-1"><legend class="text-sm ml-2 px-2">Mag Calibration</legend>
            <button class="bg-neutral-800 hover:bg-neutral-700 font-bold py-2 px-4 rounded" phx-click="m_calibration">Start</button>
        </fieldset>
        <fieldset class="grow flex flex-row justify-center border-solid border border-red-800 rounded-md p-1 my-1 ml-1"><legend class="text-sm ml-2 px-2">Accel Calibration</legend>
              <button class="bg-neutral-800 hover:bg-neutral-700 font-bold py-2 px-4 mx-2 rounded" phx-click="a_calibration_h">Hor</button>
              <button class="bg-neutral-800 hover:bg-neutral-700 font-bold py-2 px-4 mx-2 rounded" phx-click="a_calibration_v">Vert</button>
              <button class="bg-neutral-800 hover:bg-neutral-700 font-bold py-2 px-4 mx-2 rounded" phx-click="a_calibration_save">Save</button>
        </fieldset>
      </div>

      <%#div class="my-3 flex justify-center">
        <%= @message % >
      </div%>
    </fieldset>
    """
  end

  # inserisce in socket gli assign necessari al valore di default iniziale
  def mount(socket) do
    socket = socket
             |> assign(tracking_status: "Idle", target: "", target_r_asc: "N/A", target_declination: "N/A", target_az: "N/A", target_alt: "N/A")
             |> assign(r_asc: 12, declination: 45, azimuth: 30, altitude: 23)
             |> assign(message: "")
    {:ok, socket}
  end

  # riceve da LV l'update degli assign cambiati
  def update(assigns, socket) do
    # socket = socket
    #          |> assign(tracking_status: "Idle", target_r_asc: "N/A", target_declination: "N/A")
    #          |> assign(r_asc: 11, declination: 45, azimuth: 30, altitude: 23)
    #          |> assign(magcal: 1.3, maglevel: 7, magnoise: 2.1)

    {:ok, assign(socket, assigns)}
  end

end
