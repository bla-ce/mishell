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
