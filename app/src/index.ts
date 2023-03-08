import { createServer } from "http";
import { handleRequest } from "./handleRequest";

const port = Number(process.env.PORT) || 8083;
const ip = '0.0.0.0';

const server = createServer(handleRequest);

console.log("Listening on http://" + ip + ":" + port);
server.listen(port, ip);
