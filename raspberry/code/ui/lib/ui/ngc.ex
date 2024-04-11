defmodule NGC do
  use GenServer
  require Logger

  @ngc_db "NGC.csv"

  @moduledoc """
  catalogue legend
    Galaxies: G | GGroup | GPair | Gtrpl
    Star clusters: OCl | GCl | *Ass | Cl+N
    Planetary Nebulae: PN
    Nebulae: HII/Hll? | Neb | EmN | RfN | SNR
  """
    # client API

    def start_link(_state, opts \\ []) do
      Logger.info("Launching NGC GenServer")
      GenServer.start_link(__MODULE__, :ok, [name: __MODULE__])
    end

    def count(type) do
      GenServer.call(__MODULE__, {:count, type}, 10000)
    end

    # server API

    def init(:ok) do
      Logger.info("START READING NGC DB")
      {keys, objects} =
        # File.stream!(Path.join(:code.priv_dir(:ui), @ngc_db))
        File.stream!(Path.join(Application.app_dir(:ui, "priv"), @ngc_db))
        |> CSV.decode
        |> split

      state = Enum.map(objects, fn o -> map_object(keys, o) end)
      # {:ok, state, 5_000_000}
      {:ok, state}
    end

    def handle_call(:ok, _from, state) do
      {:reply, state, state}
    end

    def handle_call({:count, type}, _from, state) do
      case type do
        :messier -> {:reply, Enum.count(Enum.filter(state, fn x -> x["M"] != "" end)), state}
        :ngc -> {:reply, Enum.count(Enum.filter(state, fn x -> String.starts_with?(x["Name"], "NGC") end)), state}
        :ic -> {:reply, Enum.count(Enum.filter(state, fn x -> String.starts_with?(x["Name"], "IC") end)), state}
      end
    end

    def handle_call({:filter, type}, _from, state) do
      # https://stackoverflow.com/questions/69092526/elixir-filter-map-entries-by-a-conditional
      case type do
        :messier -> {:reply, Enum.count(Enum.filter(state, fn x -> x["M"] != "" end)), state}
        :ngc -> {:reply, Enum.count(Enum.filter(state, fn x -> String.starts_with?(x["Name"], "NGC") end)), state}
        :ic -> {:reply, Enum.count(Enum.filter(state, fn x -> String.starts_with?(x["Name"], "IC") end)), state}
      end
    end

    def handle_cast(_, state) do
      {:noreply, state}
    end

    defp split(e) do
      # elem(1) è necessario perchè CSV.decode restituisce una lista di tuple :ok [riga]
      # invece che solo [riga]
      # CSV.decode! restituirebbe [riga] ma è lazy, non funziona Enum.count(). A me serve eager
      keys = Enum.take(e, 1) |> hd |> elem(1)
      objects = for o <- Enum.take(e, -Enum.count(e)+1) do elem(o, 1) end
      {keys, objects}
    end

    defp map_object(keys, object) do
      Enum.zip(keys, object) |> Map.new
    end

end
