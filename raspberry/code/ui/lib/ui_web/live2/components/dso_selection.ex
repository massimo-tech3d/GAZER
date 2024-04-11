defmodule UiWeb.Components.DeepSkySelect2 do
  use UiWeb, :live_component

  def render(assigns) do
    ~H"""
    <div class="flex flex-col gap-2 items-center">
      <fieldset class="flex flex-col border-solid border border-red-800 rounded-md basis-full p-1 mx-1"><legend class="text-base ml-2 px-2">Choose DeepSky Object</legend>
        <.form let={f1} for={:dso} phx-submit="dso" phx-target={@myself} class="container flex flex-row items-center">
          <div class="flex flex-col">
            <div class="flex">
              <%= label f1, "Object #", class: "flex items-center px-3 pointer-events-none text-s rounded-l-md border border-red-700" %>
              <%= select(f1, :catalogue, @catalogues, [selected: @catalogue, class: "border text-s rounded-r-md  border-red-700 text-gray-500 bg-neutral-700"]) %>
              <%= text_input(f1, :dso_id, [class: "rounded-lg text-gray-800 bg-gray-500", size: "2", value: "1"])%>
              <br><br>
            </div>
            <%= submit "GO", class: "bg-neutral-800 hover:bg-neutral-700 text-s font-bold py-2 px-4 rounded my-3"%>
          </div>
        </.form>
      </fieldset>
    </div>
    """
  end

  def mount(socket) do
    dso_catalogues = ["Messier": "messier", "IC": "ic", "NGC": "ngc"]
    socket = socket
             |> assign(catalogues: dso_catalogues)
             |> assign(catalogue: "")
             |> assign(dso_id: "")

    {:ok, socket}
  end

  def update(assigns, socket) do
    {:ok, assign(socket, assigns)}
  end

  def handle_event("dso", %{"dso" => params}, socket) do
    catalogue = String.to_atom(params["catalogue"])
    oid = String.to_integer(params["dso_id"])
    send self(), {:goto_dso, catalogue, oid}
    {:noreply, socket}
  end

end
