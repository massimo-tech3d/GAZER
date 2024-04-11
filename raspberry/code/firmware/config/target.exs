import Config

config :ui, UiWeb.Endpoint,
  url: [host: "nerves.local"],
  http: [port: 80],
  cache_static_manifest: "priv/static/cache_manifest.json",
  secret_key_base: "HEY05EB1dFVSu6KykKHuS4rQPQzSHv4F7mGVB/gnDLrIu75wE/ytBXy2TaL3A6RA",
  live_view: [signing_salt: "AAAABjEyERMkxgDh"],
  check_origin: false,
  # Start the server since we're running in a release instead of through `mix`
  server: true,
  render_errors: [view: UiWeb.ErrorView, accepts: ~w(html json), layout: false],
  pubsub_server: Ui.PubSub,
  # Nerves root filesystem is read-only, so disable the code reloader
  code_reloader: false

# config :ui, Ui.Repo,
#   database: "/data/hello_phoenix/hello_phoenix_ui.db",
#   pool_size: 5,
#   show_sensitive_data_on_connection_error: true

# Use Jason for JSON parsing in Phoenix
config :phoenix, :json_library, Jason

# Use shoehorn to start the main application. See the shoehorn
# docs for separating out critical OTP applications such as those
# involved with firmware updates.

config :shoehorn,
# init: [:nerves_runtime, :nerves_pack, :nerves_init_gadget],
init: [:nerves_runtime, :nerves_pack],
app: Mix.Project.config()[:app]

# Nerves Runtime can enumerate hardware devices and send notifications via
# SystemRegistry. This slows down startup and not many programs make use of
# this feature.

config :nerves_runtime, :kernel, use_system_registry: false

# # ntp server is the phone via the "time server" app from playstore
# Possibly inteferes with the UART ?
config :nerves_time, :servers, [
  "192.168.3.2:1234",
  "192.168.3.3:1234",
  "192.168.3.4:1234",
  "192.168.3.5:1234",
  "192.168.3.6:1234",
  "192.168.3.7:1234",
  "192.168.3.8:1234",
  "192.168.3.9:1234",
  "192.168.3.10:1234",
  # "0.pool.ntp.org",
  # "1.pool.ntp.org",
  # "2.pool.ntp.org",
  # "3.pool.ntp.org"
]

# Erlinit can be configured without a rootfs_overlay. See
# https://github.com/nerves-project/erlinit/ for more information on
# configuring erlinit.

config :nerves,
  erlinit: [
    hostname_pattern: "nerves-%s"
  ]

# Configure the device for SSH IEx prompt access and firmware updates
#
# * See https://hexdocs.pm/nerves_ssh/readme.html for general SSH configuration
# * See https://hexdocs.pm/ssh_subsystem_fwup/readme.html for firmware updates

keys =
  [
    Path.join([System.user_home!(), ".ssh", "id_rsa.pub"]),
    Path.join([System.user_home!(), ".ssh", "id_ecdsa.pub"]),
    Path.join([System.user_home!(), ".ssh", "id_ed25519.pub"])
  ]
  |> Enum.filter(&File.exists?/1)

if keys == [],
  do:
    Mix.raise("""
    No SSH public keys found in ~/.ssh. An ssh authorized key is needed to
    log into the Nerves device and update firmware on it using ssh.
    See your project's config.exs for this error message.
    """)

config :nerves_ssh,
  authorized_keys: Enum.map(keys, &File.read!/1)

# Configure the network using vintage_net
# See https://github.com/nerves-networking/vintage_net for more information
config :vintage_net,
regulatory_domain: "IT",
# regulatory_domain: "00",  # globally valid regulatory domain
config: [
    {"usb0", %{type: VintageNetDirect}},
    {"eth0",
     %{
       type: VintageNetEthernet,
       ipv4: %{method: :dhcp}
     }},
    {"wlan0",
     %{
        type: VintageNetWiFi,
        vintage_net_wifi: %{
          networks: [
            %{
              mode: :ap,
              ssid: "gazer",
              key_mgmt: :none
            }
          ]
        },
        ipv4: %{
          method: :static,
          address: "192.168.3.1",
          netmask: "255.255.255.0"
        },
        dhcpd: %{
          start: "192.168.3.2",
          end: "192.168.3.10",
          options: %{
            dns: ["192.168.3.1"],
            subnet: "255.255.255.0",
            router: ["192.168.3.1"],
            domain: "gazer.az",
            search: "gazer.az"
          }
        }
      }
    }

    # REGULAR WIFI CLIENT CONNECTION - WORKS ON MY ACCESS POINT
    #  {"wlan0",
    #  %{
    #    type: VintageNetWiFi,
    #    vintage_net_wifi: %{
    #      networks: [
    #        %{
    #          key_mgmt: :wpa_psk,
    #          ssid: "Vodafone-IncredibileVisu",
    #          psk: "Minime3D"
    #         #  ssid: System.get_env("NERVES_NETWORK_SSID"),
    #         #  psk: System.get_env("NERVES_NETWORK_PSK")
    #        }
    #      ]
    #    },
    #    ipv4: %{method: :dhcp}
    #  }}
  ]

config :mdns_lite,
  # The `hosts` key specifies what hostnames mdns_lite advertises.  `:hostname`
  # advertises the device's hostname.local. For the official Nerves systems, this
  # is "nerves-<4 digit serial#>.local".  The `"nerves"` host causes mdns_lite
  # to advertise "nerves.local" for convenience. If more than one Nerves device
  # is on the network, it is recommended to delete "nerves" from the list
  # because otherwise any of the devices may respond to nerves.local leading to
  # unpredictable behavior.

  hosts: [:hostname, "nerves"],
  ttl: 120,

  # Advertise the following services over mDNS.
  services: [
    %{
      protocol: "ssh",
      transport: "tcp",
      port: 22
    },
    %{
      protocol: "sftp-ssh",
      transport: "tcp",
      port: 22
    },
    %{
      protocol: "epmd",
      transport: "tcp",
      port: 4369
    }
  ]

# Import target specific config. This must remain at the bottom
# of this file so it overrides the configuration defined above.
# Uncomment to use target specific configurations

# import_config "#{Mix.target()}.exs"
