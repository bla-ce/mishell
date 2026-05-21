- define what we want to do next, we need to define ops
    - where are new services spun up?
        - do we add service to the hardware mishell is running from?
    - do we require each hardware to be running a mishell instance?
    - where are the services defined?
    - what are the management request operations?
        - HELLO
        - AUTH
    - what are the management response operations?
        - WELCOME
        - AUTH_OK
        - OK
        - ERROR
    - what are the service request operations?
        - REGISTER
        - UNREGSITER
        - START
        - STOP
        - DESCRIBE
        - LIST
    - how do we authenticate requests?
    - how are services registered?
        - Service registers itself
        - Services are being registered by the central server
        - Services are registered by hosts

If the services are registered by the central server, it might be too difficult to host service across the local network. If the services can register themselves, we allow services to be running on different hosts, which introduce a bunch of authentication and security issues but that's definitely out of scope for now. The HELLO or AUTH request would need to specify which type of users they are. They can either be a service registry or a client trying to access one of the service.

Potentially 4 entities
    - Central server
    - Service Registries (Hosts)
    - Services
    - Client

The ultimate goal is to spin up and manage service across different networks but we'll start simple - adding service to the local network. The flow is simple, we register as hosts, we connect as a host, to then get back a token and be able to manage services. Then, we call mishell with the authentication that we've defined to spin up services that we own. For now, there is no data persistence if mishell crashes

We have two directions to go to from now on. We can focus on host authentication or service registration. I'd say we can focus on the authentication because we do have bit of ideas but none wiht service registration, let's make sure the host authentication is not taking over.

We now have a way to register and authenticate (kinda) new hosts. We are for now storing hosts' tokens inside a fixed-size array. We might need to create a host struct to be able to get more info about a particular host (IP, port, token).

We are going to define the payload format. Key value pair is fine for now
Example:
ip=192.168.1.42\n
name=grafana\n
port=3000\n

Okidoki, we have written the host struct and are adding IP to the host when being initialised. We now need to define which commands each entity will be calling to be able to set priorities and define the next steps. Understanding the requirements of each command and entity will also help us to define the format that we'll use for the payload. Current candidates are at the moment key value pairs and json. JSON will eventually become the format but I don't want to get stuck into the json implementation.

For now, a host is able to register itself using the HELLO command to ensure that Mishell is running and the AUTH command to get an id. Next step would naturally be registering a new service, which we need to define carefully. Which command? How do we define a service? For now, we do not need to worry about running the service, but maybe just adding the service to the host. We will add a new command to start them.

We can implement the REGISTER command, it will receive a payload with the service name, the service type and that should be enough to set it up. Mishell will return the REGISTER_OK command with the service payload including the id, the name, the type and the status of the service. The host will gain a service property which will be a linked list of services. We are going to need to define the service struct but that shall be fine.

For now, we keep key value pairs as above for the format. It's fine for single object, we'll see if that becomes annoying with arrays, but we shall be fine I reckon

We did not have to define a specific format for the payload as we could just put the data as defined in the code. It'll be the clients responsability to serialize the structs into meaningful and human readable format.

One thing we need to define now is how the service will be run. A first thing to define is whether a host will be running an instance of mishell. If that's the case, we'll need to know how do we set the central server, do we ever set one?

Blockchain is using a similar pattern where there is no central entity. Mishell architecture could just be several hosts communicating to each other. Each host is aware of the other hosts. Admin would be a user, not a host. That's a later question, but quite a cool one.

The next natural step is integrating users into the flow. We need a way to authenticate users, it can be as simple as what we've done for the hosts. A simple array of users that will later have permissions over specific hosts.

A user would have the same flow that a host to authenticate, the only difference would be the mode flag sent. We should be expecting a different authentication process for hosts and users.

Compared to hosts, services don't register themselves, they are registered by the host, but the incoming request can from from any entity.

Now that they are registered, we need a way to actually start them. We also need a dummy service type. We need to define a service type contract that any service would have to respect in order to be run with mishell. We can create a pong service, easy.

To query a service, you just set the mode CLIENT_TO_SERVICE. Mishell won't try to understand the command but instead, will transfer the packet to the right host.

Let's add the START command, it will accept FL_CLIENT_TO_SERVER, id of the host (fine for now, I know it counterintuitive with the no-central server plan but we need to start somewhere), the host id and the service id to run in the payload.

host=<host_id>\n
service=<service_id>\n

We have two options here, either create a start_req_t, so we do not have to any parsing, or parsing the above. I reckon the first option is best as only other instances of mishell or dedicated CLI will talk to it, no need to worry with parsing.

We don't even need to define start_req_t, just assume the payload as the right format, we already know the offsets.

We can just return OK? instead of having multiple different response ops

Right now, there is a bit of a misconception on authentication and who's allowed to register and start service, we'll get to that later.

A service type defines a set of commands that a single service can performed.

The service type struct will be as simple as a commands fixed array, a description and that's basically it. We will have to define the command struct.

No need for a command struct, they can just be pointer to functions for now.

To query a service, we need to set the new flag FL_CLIENT_TO_SERVICE. We need to find a way to include service and host id in the request. Payload would be fine but I'd like to avoid this, we are potentially wasting 4 bytes of payload. We could have a destination field inside the packet with the service id. The host is already known because the host receiving the request is responsible for the service.

Discovered a software called Consul which is quite similar in terms of managing services. There are a lot of interesting ideas that I can think about, like putting all services into maintenance mode for a single service, potential use cases and so on. The software is still different from Mishell but the idea is similar. The services are not (yet, maybe that will change?) interconnected.

We are allowing two hosts on the same ip address - Sidenote, we are storing string representation of ip address, we don't need that at all - we should have verification so that only one host can run on a single ip address.

Actually, do we want to allow the same ip address but a different port? That could work, we'd need to include the port inside the host struct, that sounds fine.

Before going further, we need to decide if Mishell will have a central server or if we use a peer-to-peer model for hosts. We also need to figure out how users will communicate with mishell.

Not having a central server is quite nice, it removes single point of failure, if the central server crashes, everything goes down with it. We don't want a whole network to depend on one entity. The peer-to-peer model sounds like a solution - we don't have a central server but instead a network of different hosts that will be responsible for their own services. A host will register itself through any available target host. The target host will be responsible for distributing the newly added host to the network. This will introduce a bunch of verification and race condition issues but we'll get to that later.

This means that each host will have a copy of the hosts array. The host array should be small enough to be sent in a packet payload. When a host wants to register itself, the target host verifies that there is enough room for another host, if yes. The host is added to the array and distributed to the other host. We'll have a race condition here because two hosts can register at the same time. We'll have to figure out how we priorise a host over another.

With this architecture in mind, we need to update the code because right now, we are registering a host with the FL_CLIENT_TO_SERVER flag which is incorrect. It is not obvious now, what the flow actually is. In the e2e tests, we are simulating a request done by another host so we'll have to implement that as well. Another thing to figure out is how do we send request to a host. It feels to me that we have to create a CLI tool for mishell.

So the flow would be the following:
    - a host is started (mishell instance)
    - a mishell CLI instance called an existing mishell instance to register the new host, specifying the ip or the port of the host
    - the mishell instance receiving the request makes some verification from the request payload and sends a HELLO to the host to be added. If a OK is returned, the host is being added to the array

A problem here is how do we reference the first host? Well, that's a problem for later but we'll find a solution

Well, that's not a problem for later, in order to test the behaviour correctly, we need to define that. We could create a command INIT. This command would allow a host to initialises the network. It won't be set as an admin host because there's none of that in this project. It will add itself to the list host and that's basically it.

A few things have become clearer about the overall vision. Mishell is a control plane and proxy. Clients never talk to services directly, they always go through mishell. Mishell finds the right host and forwards the request. The client doesn't know or care where the service physically runs.

Services are stateful. A service lives on a specific host and that host is responsible for it. If the host goes down, its services go down with it. We don't want mishell to replicate service state, that's too much. This also confirms the P2P model - we don't want a central mishell taking everything down if it crashes. Unlike Consul, we don't expect services to be interconnected. The only relationships are mishell peers talking to each other and clients querying services through mishell. That keeps the gossip layer simple - we only need to replicate host membership, not service data.

We need a CLI tool, mishell-cli. There's no other option for humans to interact with mishell without crafting raw packets. It'll be a separate binary but keeping it in assembly to stay consistent. One-shot - connect, send a packet, print the response, exit. No daemon. Default to the local unix socket, --host flag for remote. That covers the common case with no extra typing.

Now, auth is broken. The current auth was designed with a central server in mind - a host connects and gets an ID back from the central mishell. In the P2P model there's no central authority handing out IDs. Who generates the ID? Who decides if a new host is accepted? We'll need to rethink that entirely when we get to P2P.

We have two paths - fix auth and P2P first, or get a single host working end to end first. Going with single host first. The service lifecycle stuff - actually starting services, proxying, stopping - is independent of the auth model. We'll stub auth for now and the rework when we get to P2P will mostly be in ops.inc. We should keep a clear line where auth is stubbed so we know what to come back to.

The plan:
    - Phase 1: single host works end to end - fix the service_init bug, actually start a service, FL_CLIENT_TO_SERVICE routing, stop and unregister
    - Phase 2: mishell-cli
    - Phase 3: P2P - fix auth, INIT command, FL_PEER mode for mishell-to-mishell traffic, host join flow, cross-host routing
    - Phase 4: access control, users, permissions

We are not using destination. For now, a service command can't have a payload because this payload is used to get the service id. That's fine. It's just for testing, then we will implement destination.

systemd is quite similar too, cool stuff from it:
    - restart command
    - status command
    - reload command
    - last updated? To know when the service has stopped/started
    - disable / enable command
