defmodule UiWeb.Components.ScopeInfo do
  use UiWeb, :live_component
  use Phoenix.HTML
  alias UiWeb.Components
  alias UiWeb.Components.Stateless
  require Logger

  def render(assigns) do
    ~H"""
    <fieldset class="flex flex-col border-solid border border-red-800 text-sm rounded-md basis-full p-1 mx-1"><legend class="text-base ml-2 px-2">Scope</legend>
      <div class="flex flex-col">
        <div class="grid grid-cols-2">
          <fieldset class="grow border-solid border border-red-800 rounded-md p-1"><legend class="text-sm ml-2 px-2">Aim EQ</legend>
            <p>R.A: <span class="text-base font-bold"><%= @aim.pos_ar |> UiWeb.UiLive.degree_hour_format %></span></p>
            <p>Dec: <span class="text-base font-bold"><%= @aim.pos_dec |> UiWeb.UiLive.angle_format %></span></p>
          </fieldset>
          <fieldset class="grow border-solid border border-red-800 rounded-md p-1 ml-1"><legend class="text-sm ml-2 px-2">Aim AZ</legend>
            <p>Alt: <span class="text-base font-bold"><%= @aim.pos_alt |> UiWeb.UiLive.angle_format %></span></p>
            <p>Az: <span class="text-base font-bold"><%= @aim.pos_az |> UiWeb.UiLive.angle_format %></span></p>
          </fieldset>
        </div>
        <div class="grid grid-cols-2">
          <fieldset class="grow border-solid border border-red-800 rounded-md p-1"><legend class="text-sm ml-2 px-2"><%= @tracking_status %> <%= @target.label %></legend>
            <p>R.A: <span class="text-base font-bold"><%= @target.ar |> UiWeb.UiLive.degree_hour_format %></span></p>
            <p>Dec: <span class="text-base font-bold"><%= @target.dec |> UiWeb.UiLive.angle_format %></span></p>
          </fieldset>
          <fieldset class="grow border-solid border border-red-800 rounded-md p-1 ml-1"><legend class="text-sm ml-2 px-2">Target AZ</legend>
            <p>Alt: <span class="text-base font-bold"><%= @target_alt |> UiWeb.UiLive.angle_format %></span></p>
            <p>Az: <span class="text-base font-bold"><%= @target_az |> UiWeb.UiLive.angle_format %></span></p>
          </fieldset>
        </div>
      </div>
      <%#div class="flex flex-row justify-center">
        <fieldset class="flex justify-center grow self-center border-solid border border-red-800 rounded-md p-1 my-1"><legend class="text-sm ml-2 px-2">Mag Calibration</legend>
            <button class="bg-neutral-800 hover:bg-neutral-700 font-bold py-2 px-4 rounded" phx-click="calibration">Calibration</button>
        </fieldset>
        <fieldset class="flex justify-center grow self-center border-solid border border-red-800 rounded-md p-1 my-1"><legend class="text-sm ml-2 px-2">Mag Calibration</legend>
            <button class="bg-neutral-800 hover:bg-neutral-700 font-bold py-2 px-4 rounded" phx-click="m_calibration">Start</button>
        </fieldset>
        <fieldset class="grow flex flex-row justify-center border-solid border border-red-800 rounded-md p-1 my-1 ml-1"><legend class="text-sm ml-2 px-2">Accel Calibration</legend>
              <button class="bg-neutral-800 hover:bg-neutral-700 font-bold py-2 px-4 mx-2 rounded" phx-click="a_calibration_h">Hor</button>
              <button class="bg-neutral-800 hover:bg-neutral-700 font-bold py-2 px-4 mx-2 rounded" phx-click="a_calibration_v">Vert</button>
              <button class="bg-neutral-800 hover:bg-neutral-700 font-bold py-2 px-4 mx-2 rounded" phx-click="a_calibration_save">Save</button>
        </fieldset>
      </div%>

      <%#div class="my-3 flex justify-center">
        <%= @message % >
      </div%>
    </fieldset>
    """
  end

  # inserisce in socket gli assign necessari al valore di default iniziale
  def mount(socket) do
    socket = socket
             |> assign(tracking_status: "Idle", aim: %{pos_az: 30, pos_alt: 23, pos_ar: 12, pos_dec: 45})
             |> assign(target: %{label: "", ar: "N/A", dec: "N/A"}, target_alt: "N/A", target_az: "N/A")
    {:ok, socket}
  end

  # riceve da LV l'update degli assign cambiati
  def update(assigns, socket) do

    {:ok, assign(socket, assigns)}
  end

end
