defmodule Ui.AzimutComponent do
    use Phoenix.LiveComponent

    def render(assigns) do
      ~H"""
      <div class="hero"><%= @content %></div>
      """
    end

end
