defmodule Firmware.MixProject do
  use Mix.Project

  @app :firmware
  @version "0.1.0"
  @all_targets [:rpi3]

  def project do
    [
      app: @app,
      version: @version,
      elixir: "~> 1.11",
      archives: [nerves_bootstrap: "~> 1.11"],
      start_permanent: Mix.env() == :prod,
      build_embedded: true,
      deps: deps(),
      releases: [{@app, release()}],
      preferred_cli_target: [run: :host, test: :host]
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      mod: {Firmware.Application, []},
      extra_applications: [:logger, :runtime_tools, :useful]
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      # Dependencies for all targets
      {:nerves, "~> 1.7.16 or ~> 1.8.0", runtime: false},
      {:shoehorn, "~> 0.9.1"},
      {:ring_logger, "~> 0.8.5"},
      {:toolshed, "~> 0.2.26"},

      #{:astrex, git: "https://github.com/massimo-tech3d/astrex.git"},
      {:astrex, "~> 0.3.4", override: true},

      # Dependencies for all targets except :host
      {:nerves_runtime, "~> 0.13.0", targets: @all_targets},
      {:nerves_pack, "~> 0.7.0", targets: @all_targets},
      {:nerves_time, "~> 0.4.2", targets: @all_targets},
      # {:nerves_time_zones, "~> 0.1.2"},

      {:ui, path: "../ui", targets: @all_targets, env: Mix.env()},

      {:math, "~> 0.6.0"},
      {:decimal, "~> 2.0"},
      {:timex, "~> 3.7"},

      # Dependencies for specific targets
      # NOTE: It's generally low risk and recommended to follow minor version
      # bumps to Nerves systems. Since these include Linux kernel and Erlang
      # version updates, please review their release notes in case
      # changes to your application are needed.
      {:useful, "~> 0.4.0"},
      # {:nerves_system_rpi3, "~> 1.19", runtime: false, targets: :rpi3},
      {:nerves_system_rpi3, "~> 1.22", runtime: false, targets: :rpi3},
      {:circuits_uart, "~> 1.3"},
      {:circuits_gpio, "~> 0.4"},
      {:pigpiox, "~> 0.1"}
    ]
  end

  def release do
    [
      overwrite: true,
      # Erlang distribution is not started automatically.
      # See https://hexdocs.pm/nerves_pack/readme.html#erlang-distribution
      cookie: "#{@app}_cookie",
      include_erts: &Nerves.Release.erts/0,
      steps: [&Nerves.Release.init/1, :assemble],
      strip_beams: Mix.env() == :prod or [keep: ["Docs"]]
    ]
  end
end
