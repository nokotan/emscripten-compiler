import * as qs from 'querystring';
import { deflate } from 'zlib';
import { ServerResponse, IncomingMessage } from 'http';
import { build, BuildRequest, BuildResult } from './build';

function notAllowed(res: ServerResponse) {
  res.writeHead(503);
  res.end();
}

function showError(res: ServerResponse, err: Error) {
  console.log(err.toString());
  res.writeHead(501);
  res.end(`<pre>${err.toString()}</pre>`);
}

function readFormData(request: IncomingMessage, type: "json" | "form", callback: (e?: Error, r?: BuildRequest & { action: string }) => void) {
  var body = '';

  request.on('data', function (data) {
    body += data;

    // Too much POST data, kill the connection!
    // 1e6 === 1 * Math.pow(10, 6) === 1 * 1000000 ~~~ 1MB
    if (body.length > 1e6)
      request.connection.destroy();
  });

  request.on('end', function () {
    try {
      const result = type == "json" ? JSON.parse(body) : qs.parse(body);
      callback(null, result);
    } catch (ex) {
      callback(ex);
    }
  });
}

export function handleRequest(req: IncomingMessage, res: ServerResponse) {
  res.setHeader("Access-Control-Allow-Origin", "*");
  res.setHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  res.setHeader('Access-Control-Allow-Methods', 'OPTIONS, GET, POST');

  if (req.method == "OPTIONS") {
    res.writeHead(200);
    res.end();
    return;
  }

  if (req.url == "/build") {
    if (req.method != "POST") return notAllowed(res);
    readFormData(req, "json", (err: Error, input: BuildRequest) => {
      if (err) return showError(res, err);
      build(input, (err, result) => {
        if (err) return showError(res, err);
        res.setHeader('Content-type', 'application/json; charset=utf-8');
        res.setHeader('Content-Encoding', 'deflate');
        res.writeHead(200);
        const responseText = JSON.stringify(result);
        const responseBuffer = Buffer.from(responseText, 'utf8');
        
        deflate(responseBuffer, (_, deflatedBuffer) => res.end(deflatedBuffer));
      });
    });
    return;
  }

  res.writeHead(404);
  res.end();
};
