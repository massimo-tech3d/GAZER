defmodule Firmware.Application do
  # See https://hexdocs.pm/elixir/Application.html
  # for more information on OTP Applications
  @moduledoc false

  use Application

  @impl true
  def start(_type, _args) do
    # See https://hexdocs.pm/elixir/Supervisor.html
    # for other strategies and supported options
    opts = [strategy: :one_for_one, name: Firmware.Supervisor]

    children =
      [
        # Children for all targets
        # Starts a worker by calling: Firmware.Worker.start_link(arg)
        # {Firmware.Worker, arg},
        {Phoenix.PubSub, name: :sensors}

      ] ++ children(target())

    Supervisor.start_link(children, opts)
  end

  # List all child processes to be supervised
  def children(:host) do
    [
      # Children that only run on the host
      # Starts a worker by calling: Firmware.Worker.start_link(arg)
      # {Firmware.Worker, arg},
    ]
  end

  def children(_target) do
    [
      # Children for all targets except host
      # Starts a worker by calling: Firmware.Worker.start_link(arg)
      # {Firmware.Worker, arg},
      # Supervisor.child_spec(Engine, id: Engine, restart: :permanent),
      # Supervisor.child_spec(Steppers, id: Steppers, restart: :permanent)
      # Engine
      {Engine, [:ok]}
      # {Steppers, [:ok]}  # enable if when moving to the genserver version of stepper module.
    ]
  end

  def target() do
    Application.get_env(:firmware, :target)
  end
end
