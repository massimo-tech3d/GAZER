defmodule UiWeb.Components.Calibration do
  use UiWeb, :live_component
  use Phoenix.HTML
  alias UiWeb.Components
  alias UiWeb.Components.Stateless
  require Logger

  def render(assigns) do
    ~H"""
    <%#fieldset class="flex flex-col border-solid border border-red-800 text-sm rounded-md basis-full p-1 mx-1">
      <legend class={"text-base ml-2 px-2 #{cal_ready(@u_hiz_r, @u_hiz, @u_zero, @m_cal)}"}>Calibration</legend%>
    <fieldset class="flex flex-col basis-full p-1 mx-1">
    <%# div class="hidden"%>
    <div class="flex flex-col justify-center p-1 mx-1">
        <%= if @theta do %>
          <%= if @theta == 0 do %>
            <div class="flex flex-col items-center grow border-solid border border-green-800 text-sm text-green-800 rounded-md p-1 my-1 ml-1">
              <p class="">The mount is perfectly level !</p>
              <p class="">GOTO should be accurate</p>
              <p class="mt-2">(keep phone and metal parts away from the sensor block)</p>
            </div>
          <% end %>
          <%= if @theta > 0 and @theta <= 5 do %>
            <div class="flex flex-col items-center grow border-solid border border-green-800 text-sm text-green-800 rounded-md p-1 my-1 ml-1">
              <p class="">The mount is off-level by only <span class="text-lg font-black"><%= @theta %>°</span></p>
              <p class="">Software will compensate this.</p>
              <p class="">GOTO should be accurate</p>
              <p class="mt-2">(keep phone and metal parts away from the sensor block)</p>
            </div>
          <% end %>
          <%= if @theta > 5 and @theta <= 10 do %>
            <div class="flex flex-col items-center grow border-solid border border-yellow-800 text-sm text-yellow-800 rounded-md p-1 my-1 ml-1">
              <p class="">The mount is off-level by <span class="text-lg font-black"><%= @theta %>°</span></p>
              <p class="">Software will mostly compensate this</p>
              <p class="">GOTO might be slightly inaccurate but still useful</p>
              <p class="mt-2">(keep phone and metal parts away from the sensor block)</p>
            </div>
          <% end %>
          <%= if @theta > 10 do %>
            <div class="flex flex-col items-center border-solid border border-red-500 bg-red-900 text-sm text-black rounded-md p-1 my-1 ml-1">
              <p class="">The mount is off-level by <span class="text-lg font-black"><%= @theta %>°</span></p>
              <p class="">This is a lot, software may NOT be able to fully compensate</p>
              <p class="font-black mt-2">IT IS ADVISED TO BETTER LEVEL THE MOUNT</p>
              <%# popup %>
            </div>
          <% end %>
        <% end %>

        <fieldset class="flex flex-col justify-center grow border-solid border border-red-800 text-sm rounded-md p-1 my-1 ml-1"><legend class="text-sm ml-2 px-2">Sensor block</legend>
            <p class="text-sm text-red-800 mb-2 mx-5">Specify if the magnetometer block is mounted horizontally or vertically (<span class="font-black">default is horizontal</span>)</p>
            <div class="flex flex-row justify-center grow text-sm p-1 my-1 ml-1">
              <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-sm #{text_color(@block_h)} py-2 px-10 m-1 rounded"} phx-click="sensor_block_horizontal">Horizontal</button>
              <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-sm #{text_color(@block_v)} py-2 px-10 m-1 rounded"} phx-click="sensor_block_vertical">Vertical</button>
            </div>
        </fieldset>
        <div class="flex flex-row justify-center">
          <%# <fieldset class="flex flex-col justify-center grow border-solid border border-red-800 text-sm rounded-md p-1 my-1"><legend class="text-sm ml-2 px-2">Untilter</legend>
            <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-sm #{text_color(@u_hiz_r)} py-2 px-4 m-1 rounded"} phx-click="untilt_hiz_r">Reversed</button>
            <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-sm #{text_color(@u_hiz)} py-2 px-4 m-1 rounded"} phx-click="untilt_hiz">Flat</button>
            <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-sm #{text_color(@u_zero)} py-2 px-4 m-1 rounded"} phx-click="untilt_zero">Zero</button>
          </fieldset> %>
          <fieldset class="flex flex-col justify-center grow border-solid border border-red-800 text-sm #{text_color(@calibration.u_hiz_r)} rounded-md p-1 my-1 ml-1"><legend class="text-sm ml-2 px-2">Magnetometer</legend>
            <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-sm #{text_color(@u_zero)} py-2 px-4 m-1 rounded"} phx-click="untilt_zero">Zero untilter</button>
            <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-sm #{text_color(@m_cal)} py-2 px-4 m-1 rounded #{button_disabled(@m_cal)}"} phx-click="m_calibration">Cal</button>
            <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-sm #{text_color(@m_comp)} py-2 px-4 m-1 rounded #{button_disabled(@m_comp)}"} phx-click="m_compensation">Comp (optional)</button>
          </fieldset>
          <fieldset class="flex flex-col justify-center grow border-solid border border-red-800 text-sm #{text_color(@calibration.u_hiz_r)} rounded-md p-1 my-1 ml-1"><legend class="text-sm ml-2 px-2">Accelerometer</legend>
            <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-sm #{text_color(@a_cal_v)} py-2 px-4 m-1 rounded"} phx-click="a_calibration_v">Vert</button>
            <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-sm #{text_color(@a_cal_h)} py-2 px-4 m-1 rounded"} phx-click="a_calibration_h">Hor</button>
            <button class={"bg-neutral-800 hover:bg-neutral-700 font-bold text-gray-500 text-sm py-2 px-4 m-1 rounded"} phx-click="a_calibration_save">Save</button>
          </fieldset>
        </div>

        <button class="btn" onclick="help_modal.showModal()">Calibration Help</button>
        <dialog id="help_modal" class="modal modal-bottom sm:modal-middle bg-black border border-2 border-red-900 rounded-md">
        <div class="modal-box bg-black text-red-800 border border-0">
            <h3 class="font-bold text-lg">How to properly calibrate</h3>
            <p class="py-4">a proper calibration is mandatory before the sensors can be trusted - proceed in the proposed order</p>
            <p><b>Untilter - Mandatory</b></p>
            <%#p>1 - Flip the magnetometer upside down and tap <span class="border border-red-900 rounded-md px-1">Reversed</span></p>
            <p>2 - Replace magnetometer in normal position and tap <span class="border border-red-900 rounded-md px-1">Flat</span></p%>
            <p>Ensure the magnetometer block in as flat as possible and tap <span class="border border-red-900 rounded-md px-1">Zero</span></p>
            <p class="mt-2"><b>Magnetometer</b></p>
            <p>1 - Calibration  - <b>Mandatory</b> tap <span class="border border-red-900 rounded-md px-1">Cal</span></p>
            <p>2 - Altitude compensation - <b>Optional - not needed if your OTA is not ferromagnetic</b> - make sure the OTA is vertical and tap <span class="border border-red-900 rounded-md px-1">Comp (optional)</span></p>
            <p class="mt-2"><b>Accelerometer</b></p>
            <p>This is optional. Can be saved for future observations. No need to be repeat unless the OTA is removed form the mount or if the sensor is removed</p>
            <p>1 - Vertical  - place the OTA perfectly vertical and tap <span class="border border-red-900 rounded-md px-1">Vert</span></p>
            <p>2 - Horizontal  - place the OTA perfectly horizontal and tap <span class="border border-red-900 rounded-md px-1">Hor</span></p>
            <p>3 - Save  - tap <span class="border border-red-900 rounded-md px-1">Save</span> to save the Accelerometer calibration for future observing sessions</p>
            <div class="flex modal-action my-2 justify-center">
              <form method="dialog">
                <!-- if there is a button in form, it will close the modal -->
                <button class="font-bold text-sm py-2 px-4 mt-1 border border-red-500 rounded text-red-500">Close</button>
              </form>
            </div>
          </div>
        </dialog>
      </div>
    </fieldset>
    """
  end

  ######################
  # TODO
  # implement status - change button color and disable once done any calibration
  # highlight buttons for calibrations that are mandatory - basically m_calibration
  #
  # hide calibration block - place a button to unhide it - color/text of button should show calibration status (completed vs to be done)
  ######################

  # inserisce in socket gli assign necessari al valore di default iniziale
  def mount(socket) do
    socket = socket
             |> assign(tracking_status: "Idle", aim: %{pos_az: 30, pos_alt: 23, pos_ar: 12, pos_dec: 45})
             |> assign(target: %{label: "", ar: "N/A", dec: "N/A"}, target_alt: "N/A", target_az: "N/A")
             |> assign(u_hiz_r: :nok, u_hiz: :nok, u_zero: :nok, m_cal: :nok, m_comp: :nok, a_cal_v: :nok, a_cal_h: :nok)
             |> assign(block_h: :ok, block_v: :nok, theta: false)
    {:ok, socket}
  end

  # riceve da LV l'update degli assign cambiati
  def update(assigns, socket) do
    Logger.debug(assigns)
    {:ok, assign(socket, assigns)}
  end

  defp text_color(id) do
    case id do
      :nok -> "text-red-800"
      :progress -> "text-gray-500"
      _ -> "text-green-800"
    end
  end

  defp button_disabled(id) do
    if (id == :ok) do
      "cursor-not-allowed"
    end
  end

  defp cal_ready(hzr, hz, uzero, mcal) do
    if hzr == :ok and hz == :ok and uzero == :ok and mcal == :ok do
      "text-green-800"
    else
      "text-red-800"
    end
  end
end
