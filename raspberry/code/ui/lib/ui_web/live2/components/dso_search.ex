defmodule UiWeb.Components.DeepSkySearch2 do
  use UiWeb, :live_component

  def render(assigns) do
    ~H"""
    <div class="flex flex-col gap-2 items-center">
      <fieldset class="flex flex-col border-solid border border-red-800 rounded-md basis-full p-1 mx-1"><legend class="text-base ml-2 px-2">DeepSky Filter</legend>
        <.form let={f} for={:dso_specs} phx-submit="dso_go" phx-change="dso_filter" phx-target={@myself} class="container flex flex-col items-center">
          <%= label f, :limit_magnitude, class: "text-s" %>
            <fieldset class="space-y-1 text-s sm:w-60 text-red-500">
              <%= range_input(f, :max_mag, [class: "w-full accent-red-800", min: "9", max: "17", step: "1", value: @max_mag]) %>
              <div aria-hidden="true" class="flex justify-between px-1">
                <span>9</span>
                <span>11</span>
                <span>13</span>
                <span>15</span>
                <span class="text-xl">∞</span>
              </div>
            </fieldset>
            <div>
              <%= label f, :messier_only, class: "text-s" %>
              <%= checkbox(f, :messier_only, [class: "rounded-md focus:ring-0 bg-gray-500 accent-emerald-600 my-2", checked: @messier_only=="true"]) %>
            </div>
            <div class="flex flex-col">
              <div class="flex mt-3">
                <span class="flex items-center px-3 pointer-events-none text-s rounded-l-md border border-red-700">Object type</span>
                <%= select(f, :dso_filter, @choices, [selected: @dso_type, class: "border text-s rounded-r-md border-red-700 text-gray-500 bg-neutral-700"]) %>
              </div>
              <%= select(f, :object, @objects, [objects: @object, class: "border mt-3 text-s rounded-md border-red-700 text-gray-500 bg-neutral-700"]) %>
              <%= submit "Go", class: "bg-neutral-800 hover:bg-neutral-700 text-s font-bold py-2 px-4 rounded my-3"%>
            </div>
        </.form>
      </fieldset>
    </div>
    """
  end

  def mount(socket) do
    dso_types = [:any, :globular_clusters, :open_clusters, :galaxies, :nebulas]
    socket = socket
             |> assign(choices: dso_types)
             |> assign(dso_type: "any")
             |> assign(messier_only: false)
             |> assign(max_mag: 13)
             |> assign(matching: [])
             |> assign(objects: [])
             |> assign(object: [])

    {:ok, socket}
  end

  def update(assigns, socket) do
    header = ["Apply filters until you're happy": 0]
    matching_objects =
      if Map.has_key?(assigns, :objects) do
        header ++ assigns[:objects]
      else
        ["Apply filters until you're happy": 0]
      end
    assigns = Map.put(assigns, :objects, matching_objects)

    {:ok, assign(socket, assigns)}
  end

  def handle_event("dso_filter", %{"dso_specs" => params}, socket) do
    send self(), {:search_dso, params}
    {:noreply, socket}
  end

  def handle_event("dso_go", %{"dso_specs" => params}, socket) do
    oid = params["object"]
    if oid != "0" do
      {cat, id} =
        case String.first(params["object"]) do
          "I" -> {:ic, String.to_integer(String.slice(oid, 2..10))}
          "N" -> {:ngc, String.to_integer(String.slice(oid, 3..10))}
          # "M" -> {:messier, String.to_integer(String.slice(oid, 1..10))}
        end
      send self(), {:goto_dso, cat, id}
    end
    {:noreply, socket}
  end

end
