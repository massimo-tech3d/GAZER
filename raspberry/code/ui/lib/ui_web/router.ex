defmodule UiWeb.Router do
  use UiWeb, :router

  pipeline :browser do
    plug :accepts, ["html"]
    plug :fetch_session
    plug :fetch_flash
    plug :fetch_live_flash
    plug :protect_from_forgery
    plug :put_secure_browser_headers
    plug :put_root_layout, {UiWeb.LayoutView, :root}
  end

  pipeline :api do
    plug :accepts, ["json"]
  end

  scope "/", UiWeb do
    pipe_through :browser

    live "/", UiLive
    get "/config", PageController, :config
    get "/motor_speed", PageController, :motor_speed
    post "/speed_test", PageController, :speed_test
    post "/speed_test/stop", PageController, :stop_test
    post "/speed_test/end", PageController, :speed_test_end
  end

  scope "/config", UiWeb do
    pipe_through :browser

    get "/", PageController, :config
    post "/update", PageController, :update_configuration
    post "/factory_reset", PageController, :factory_reset
  end

  # Other scopes may use custom stacks.
  # scope "/api", UiWeb do
  #   pipe_through :api
  # end

  # Enables LiveDashboard only for development
  #
  # If you want to use the LiveDashboard in production, you should put
  # it behind authentication and allow only admins to access it.
  # If your application does not have an admins-only section yet,
  # you can use Plug.BasicAuth to set up some basic authentication
  # as long as you are also using SSL (which you should anyway).
  if Mix.env() in [:dev, :test] do
    import Phoenix.LiveDashboard.Router

    scope "/" do
      pipe_through :browser

      live_dashboard "/dashboard", metrics: UiWeb.Telemetry
    end
  end
end
