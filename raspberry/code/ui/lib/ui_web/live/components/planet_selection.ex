defmodule UiWeb.Components.PlanetSelection do
  use UiWeb, :live_component

  def render(assigns) do
    ~H"""
    <div class="flex flex-col gap-2 items-center">
      <%#fieldset class="flex flex-col border-solid border border-red-800 rounded-md basis-full p-1 mx-1"><legend class="text-base ml-2 px-2">Choose Planet</legend%>
      <fieldset class="flex flex-col basis-full p-1 mx-1">
      <.form let={f} for={:pianeta} phx-submit="planet" phx-target={@myself} class="container flex flex-col items-center">
          <div class="flex flex-col">
            <div class="flex">
            <span class="flex items-center px-3 pointer-events-none text-s rounded-l-md border border-red-700">Planet</span>
              <%= select(f, :planet, @choices, [prompt: "Choose planet", selected: @selected_planet, class: "border text-s rounded-r-md focus:ring-inset border-red-700 text-gray-500 bg-neutral-700 focus:ring-emerald-600"]) %>
            </div>
            <%= submit "GO", class: "bg-neutral-800 hover:bg-neutral-700 text-s font-bold py-2 px-4 rounded my-3"%>
          </div>
        </.form>
      </fieldset>
    </div>
    """
  end

  def mount(socket) do
    planets = [:mercury, :venus, :moon, :mars, :jupiter, :saturn, :uranus, :neptune, :pluto]
    socket = socket
             |> assign(choices: planets)
             |> assign(selected_planet: "")

    {:ok, socket}
  end

  def update(assigns, socket) do
    socket = socket
             |> assign(selected_planet: "")

    {:ok, assign(socket, assigns)}
  end

  # @doc """
  # gestisce l'evento del bottone "GO" premuto in Planets
  # Live update della pagina
  # """
  def handle_event("planet", %{"pianeta" => params}, socket) do
    if params["planet"] != "" do
      send self(), {:goto_planet, params["planet"]} |> IO.inspect()
    end
    {:noreply, socket}
  end

end
