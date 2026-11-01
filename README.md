# Distributed realm-trading system in C

A peer-to-peer trading network written in C for Linux as the final project of the Operating Systems course at La Salle – Universitat Ramon Llull (2025–26). It was built in a team of two by **Jon Ballanca** and **Alejandro Cortés**.

Each node (a *Maester*) runs one "realm" with its own product inventory. Realms find each other through a routing table, form alliances by exchanging sigil images, and then trade goods with each other. All of it runs over a binary protocol we designed ourselves on top of TCP.

## What it shows

- **Networking:** a TCP server and client on each node. Requests to realms the node can't reach directly are forwarded through its routing table, with a default route as fallback.
- **Custom wire protocol:** fixed 320-byte frames with type, origin, destination, length, data, padding and a checksum. The protocol covers ACKs and NACKs, pings, alliance requests and responses, product listings, orders and disconnects.
- **Reliable file transfer:** images and inventory files are split across many frames, reassembled on the receiving side and verified end-to-end with an MD5 checksum.
- **Concurrency:** a pthread accepts and handles incoming connections while the main thread runs an interactive command shell. Shared terminal output is protected with mutexes.
- **Processes and IPC:** a pool of *envoy* worker processes is created with `fork`, and they talk to the parent over pipes. The parent multiplexes those pipes with `select`. `fork`, `execlp` and `dup2` are used to call system tools such as `md5sum`.
- **Signals:** `SIGINT` triggers a clean shutdown that notifies allied realms and frees resources, and `SIGALRM` drives timeouts.

## Commands

```
LIST REALMS                       realms reachable from this node
PLEDGE <realm> <sigil.png>        request an alliance and send your sigil
PLEDGE RESPOND <realm> ACCEPT|REJECT
PLEDGE STATUS                     state of every alliance request
LIST PRODUCTS [<realm>]           your own inventory, or an ally's
START TRADE <realm>               open a trade (add / remove / send / cancel)
ENVOY STATUS
EXIT
```

## Build and run

```bash
make
./SO_Project.exe <config_file> <inventory_file>
```

The config file sets the realm name, its working folder, the number of envoys, its IP and port, and a `--- ROUTES ---` table that maps realm names to next-hop addresses. `maester.dat`, `maester1.dat` and `maester2.dat` are example configs for running three nodes, and the `*_stock.db` files are binary inventories.

## Layout

| File | Responsibility |
|---|---|
| `maester.c` | Entry point, command shell, connection-handling thread |
| `communication.c` | Routing, alliances, frame reception and reassembly |
| `protocols.c` | Frame encoding and decoding, checksums, MD5 |
| `envoy.c` | Envoy worker processes |
| `inventory.c`, `config.c` | Inventory and config file parsing |
| `commands.c`, `printing.c` | Command parsing and terminal output |
