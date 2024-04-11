defmodule Gps do
  require Logger
  use GenServer

  @gpsd_port 11111
  @contact_port 2947

  def start_link() do
    Logger.info("STARTING GPS RECEIVER")
    GenServer.start_link(__MODULE__, @gpsd_port)
  end

  def init(gpsd_port) do
    # Use erlang's `gen_udp` module to open a socket
    # With options:
    #   - binary: request that data be returned as a `String`
    #   - active: gen_udp will handle data reception, and send us a message `{:udp, socket, address, port, data}` when new data arrives on the socket
    # Returns: {:ok, socket}
    :gen_udp.open(@gpsd_port, [:binary, active: true])
  end

  def terminate(_, _) do
    Logger.info("GPS TERMINATING")
  end

  def handle_info({:udp, _socket, _address, _port, data}, socket) do
    handle_packet(data, socket)
  end

  defp handle_packet("quit\n", socket) do
    Logger.info("Received: quit")
    :gen_udp.close(socket)
    {:stop, :normal, :nil}
  end

  defp handle_packet(data, socket) do
    Logger.info("Received data")
    String.trim(data)
    |> String.split(",")
    |> parse_message
    |> send_back(socket)
  end

  defp parse_message(["$GNRMC" | fields]) do
    case Enum.at(fields,1) do
      "A" -> parse_fields_GNRMC(fields)
      _ ->  Logger.info("Sat not fixed yet")
            {:error, "no fix"}
    end
  end

  defp parse_message(mex) do
    {:error, "no message:"}
  end

  defp parse_fields_GNRMC(fields) do
    {:ok, %{}}
    |> get_utc(Enum.at(fields, 0), Enum.at(fields, 8))
    |> get_lat(Enum.at(fields, 2), Enum.at(fields, 3))
    |> get_lon(Enum.at(fields, 4), Enum.at(fields, 5))
  end

  defp send_back({:error, error}, socket) do
    Logger.info(error)
    {:noreply, socket}
  end

  defp send_back({:ok, data}, socket) do
    Logger.info("Sending back gps data")
    GenServer.cast(Engine, {:gps, data})
    {:stop, :normal, :nil}
  end

  defp get_utc({:error, res}, utc, day) do
    {:error, res}
  end

  defp get_utc({:ok, res}, utc, day) do
    {hh, utc} = String.split_at(utc, 2)
    {min, ss} = String.split_at(utc, 2)
    ss = hd(String.split(ss, "."))
    {dd, day} = String.split_at(day, 2)
    {mm, yy} = String.split_at(day, 2)
    try do
      {success, dt} = Timex.parse("20#{yy}-#{mm}-#{dd}T#{hh}:#{min}:#{ss}", "{ISO:Extended}")
      {success, Map.put(res, :utc, dt)}
    rescue
      _ -> Logger.error("Error parsing UTC")
           {:error, "error parsing UTC day#{day}time#{utc}"}
    end
  end

  defp get_lat({:error, res}, lat, ns) do
    {:error, res}
  end

  defp get_lat({:ok, res}, lat, ns) do
    {deg, min} = String.split_at(lat, 2)
    sign = case ns do
             "N" -> 1
             "S" -> -1
             _   -> 0
           end
    try do
      {dd, _opt} = Float.parse(deg)
      {mm, _opt} = Float.parse(min)
      ddec = mm / 60
      {:ok, Map.put(res, :lat, sign * (dd + ddec))}
    rescue
      _ -> Logger.error("Error parsing lat")
           {:error, "error parsing lat"}
    end
  end

  defp get_lon({:error, res}, lon, _ew) do
    {:error, res}
  end

  defp get_lon({:ok, res}, lon, ew) do
    Logger.info("extracting LONGITUDE #{lon}")

    {deg, min} = String.split_at(lon, 3)
    sign = case ew do
      "E" -> -1  # note, check and delete comment - fixed bug E was 1 instead of -1
      "W" -> 1   # note, check and delete comment - fixed bug W was -1 instead of 1
      _   -> 0
    end
    try do
      {dd, _opt} = Float.parse(deg)
      {mm, _opt} = Float.parse(min)
      ddec = mm / 60
      {:ok, Map.put(res, :lon, sign * (dd + ddec))}
    rescue
      _ -> Logger.error("Error parsing lon")
           {:error, "error parsing lon"}
    end
  end
end

# discard messages not structured like this
# $GNRMC,164848.646,A,4529.5996,N,00911.0383,E,0.000,161.26,030922,,,A,V*39
# where:
# -   $GNRMC
# 0   164847.651   - UTC 16h 48m 47s
# 1   A/V          - Status A=Valid  V=Warning   -- if V --> no valid/accurate fix
# 2-3 4529.5996,N  - LAT 45°29.5996' North (S = south)
# 4-5 00911.0383,E - LON 009°11.0383' East (W = West)
# 6   ignore
# 7   ignore
# 8   ddmmyy       - Date
# 10  degrees      - Magnetic variation. To be summed to the magnetometer reading to get true north
# 11-13 ignore
