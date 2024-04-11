defmodule Ui.AzimutComponent2 do
    use Phoenix.LiveComponent

    def render(assigns) do
      ~H"""
      <div class="hero"><%= @content %></div>
      """
    end

end
