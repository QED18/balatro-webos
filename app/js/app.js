(function () {
  "use strict";

  var W = 1600, H = 900;
  var canvas = document.getElementById("game");
  var ui = document.createElement("canvas");
  ui.width = W;
  ui.height = H;
  var ctx = ui.getContext("2d", { alpha: true });
  var pointer = { x: -999, y: -999, down: false };
  var flash = 0;
  var shake = 0;
  var particles = [];
  var actions = [];
  var start = performance.now();
  var lastFrame = start;
  var lastUiPaint = 0;

  var query = new URLSearchParams(location.search);
  var portal = query.get("portal") || "http://TV-IP:47811";
  var pin = /^\d{4}$/.test(query.get("pin") || "") ? query.get("pin") : "— — — —";

  function rect(x, y, w, h, r) {
    ctx.beginPath();
    ctx.moveTo(x + r, y);
    ctx.arcTo(x + w, y, x + w, y + h, r);
    ctx.arcTo(x + w, y + h, x, y + h, r);
    ctx.arcTo(x, y + h, x, y, r);
    ctx.arcTo(x, y, x + w, y, r);
    ctx.closePath();
  }

  function panel(x, y, w, h, fill, border, r) {
    ctx.fillStyle = "#03120e";
    rect(x + 7, y + 8, w, h, r);
    ctx.fill();
    ctx.fillStyle = fill;
    rect(x, y, w, h, r);
    ctx.fill();
    ctx.lineWidth = 4;
    ctx.strokeStyle = border;
    ctx.stroke();
    ctx.save();
    ctx.globalAlpha = 0.12;
    ctx.fillStyle = "#eaffdc";
    for (var iy = y + 5; iy < y + h - 5; iy += 8) for (var ix = x + 5; ix < x + w - 5; ix += 8) ctx.fillRect(ix, iy, 1, 1);
    ctx.restore();
  }

  function text(value, x, y, size, color, align, font) {
    ctx.font = (font || "800") + " " + size + "px " + (font ? "Georgia,serif" : "Trebuchet MS,Arial,sans-serif");
    ctx.textAlign = align || "left";
    ctx.textBaseline = "alphabetic";
    ctx.fillStyle = "#061c16";
    ctx.fillText(value, x + 3, y + 4);
    ctx.fillStyle = color;
    ctx.fillText(value, x, y);
  }

  function small(value, x, y, color) {
    ctx.font = "900 15px Trebuchet MS,Arial,sans-serif";
    ctx.textAlign = "left";
    ctx.textBaseline = "alphabetic";
    ctx.letterSpacing = "2px";
    ctx.fillStyle = color || "#a8e2c6";
    ctx.fillText(value, x, y);
    ctx.letterSpacing = "0px";
  }

  function wrap(value, x, y, width, line, color, font) {
    ctx.font = font || "700 22px Trebuchet MS,Arial,sans-serif";
    ctx.textAlign = "left";
    ctx.textBaseline = "alphabetic";
    ctx.fillStyle = color || "#eaffef";
    var words = value.split(" "), row = "", yy = y;
    for (var i = 0; i < words.length; i++) {
      var next = row ? row + " " + words[i] : words[i];
      if (ctx.measureText(next).width > width && row) {
        ctx.fillText(row, x, yy);
        yy += line;
        row = words[i];
      } else row = next;
    }
    if (row) ctx.fillText(row, x, yy);
    return yy;
  }

  function card(x, y, angle, rank, suit, suitColor, t, hover) {
    var floatY = Math.sin(t * 1.1 + x * 0.014) * 5;
    ctx.save();
    ctx.translate(x, y + floatY);
    ctx.rotate(angle + (hover ? Math.sin(t * 3) * 0.025 : 0));
    panel(-48, -68, 96, 136, "#f6efd6", "#102c24", 9);
    ctx.fillStyle = suitColor;
    ctx.font = "900 27px Georgia,serif";
    ctx.fillText(rank, -33, -36);
    ctx.font = "900 43px Georgia,serif";
    ctx.fillText(suit, -28, 23);
    ctx.globalAlpha = hover ? 0.56 : 0.19;
    var holo = ctx.createLinearGradient(-50, -50, 50, 50);
    holo.addColorStop(0, "#00eaff"); holo.addColorStop(.45, "#ff59c9"); holo.addColorStop(1, "#ffe35f");
    ctx.fillStyle = holo;
    rect(-43, -63, 86, 126, 7); ctx.fill();
    ctx.restore();
  }

  function qr(x, y, size) {
    var cells = 17, d = size / cells;
    ctx.fillStyle = "#fff4c7"; ctx.fillRect(x, y, size, size);
    function finder(cx, cy) {
      ctx.fillStyle = "#123a2e"; ctx.fillRect(x + cx * d, y + cy * d, d * 5, d * 5);
      ctx.fillStyle = "#fff4c7"; ctx.fillRect(x + (cx + 1) * d, y + (cy + 1) * d, d * 3, d * 3);
      ctx.fillStyle = "#123a2e"; ctx.fillRect(x + (cx + 2) * d, y + (cy + 2) * d, d, d);
    }
    for (var row = 0; row < cells; row++) for (var col = 0; col < cells; col++) {
      var inFinder = (col < 5 && row < 5) || (col > 11 && row < 5) || (col < 5 && row > 11);
      if (!inFinder && ((row * 13 + col * 7 + row * col) % 5 < 2)) {
        ctx.fillStyle = "#123a2e"; ctx.fillRect(x + col * d, y + row * d, d - 1, d - 1);
      }
    }
    finder(0, 0); finder(12, 0); finder(0, 12);
  }

  function hit(x, y, w, h) { return pointer.x >= x && pointer.x <= x + w && pointer.y >= y && pointer.y <= y + h; }

  function addParticles() {
    for (var i = 0; i < 18; i++) particles.push({ x: 798, y: 514, vx: (Math.random() - .5) * 600, vy: -Math.random() * 360, age: 0, color: i % 2 ? "#e4cb5e" : "#8debd0" });
  }

  function drawTooltip(tip) {
    var w = Math.min(335, ctx.measureText(tip).width + 38), x = Math.min(W - w - 25, pointer.x + 20), y = Math.max(24, pointer.y - 60);
    panel(x, y, w, 42, "#f8f1d5", "#0e382d", 7);
    ctx.font = "800 14px Trebuchet MS,Arial,sans-serif";
    ctx.fillStyle = "#173f33"; ctx.fillText(tip, x + 18, y + 27);
  }

  function drawUI(t, delta) {
    ctx.clearRect(0, 0, W, H);
    actions = [];
    var titleScale = 1 + flash * .035;
    ctx.save();
    ctx.translate(90, 106);
    ctx.scale(titleScale, titleScale);
    text("BALATRO", 0, 0, 98, "#cef9ea", "left", "900");
    text("for webOS", 40, 54, 49, "#f4d682", "left", "900");
    ctx.restore();
    small("♠  COMMUNITY HOMEBREW  ·  YOUR OWN COPY  ♥", 95, 193, "#a7e4cd");
    wrap("Pair your legally purchased Steam copy over your private home network. Nothing is uploaded to a cloud service.", 95, 238, 535, 33, "#e7f7ef");
    panel(90, 377, 555, 96, "#0b493f", "#c3eee0", 13);
    ctx.fillStyle = "#e5cd61"; ctx.beginPath(); ctx.arc(123, 425, 18, 0, Math.PI * 2); ctx.fill();
    text("♦", 123, 434, 22, "#a8394a", "center", "900");
    text("PRIVATE BY DESIGN", 160, 417, 17, "#f3fff7");
    wrap("A short PIN pairs your computer with this TV for setup only.", 160, 444, 440, 23, "#d6eee5", "700 16px Trebuchet MS,Arial,sans-serif");
    card(177, 650, -.18, "J", "♣", "#17483d", t, hit(125, 575, 104, 150));
    card(252, 632, .01, "Q", "♥", "#a33a48", t, hit(200, 557, 104, 150));
    card(327, 649, .17, "K", "♠", "#17483d", t, hit(275, 574, 104, 150));

    panel(710, 100, 795, 690, "#164a40", "#d0f4e6", 20);
    small("01 / PAIR", 756, 151);
    text("Open the setup portal", 755, 215, 52, "#f1fff8", "left", "900");
    panel(1309, 123, 135, 40, "#ad3d4b", "#491c27", 6);
    text("PREVIEW", 1376, 150, 14, "#fff9d7", "center");
    wrap("Use the address on your computer, or scan the QR code. The four-digit PIN remains on the TV.", 756, 257, 615, 28, "#d6eee5");

    panel(755, 350, 332, 195, "#edf7e7", "#092b23", 11);
    small("LOCAL ADDRESS", 782, 389, "#3f8875");
    ctx.font = "900 25px ui-monospace,Menlo,monospace"; ctx.fillStyle = "#164b3e";
    wrap(portal, 782, 430, 270, 29, "#164b3e", "900 24px ui-monospace,Menlo,monospace");

    panel(1080, 350, 210, 195, "#e7cf65", "#092b23", 11);
    qr(1133, 372, 104);
    text("SCAN TO OPEN", 1185, 507, 13, "#214337", "center");
    text("♠", 1185, 531, 17, "#ae3b49", "center", "900");
    actions.push({ x: 1080, y: 350, w: 210, h: 195, label: "QR preview: the real build encodes the local address only" });

    panel(755, 582, 535, 109, "#edf7e7", "#092b23", 11);
    small("PAIRING PIN", 782, 620, "#3f8875");
    text(pin.split("").join(" "), 782, 665, 31, "#a83b4a", "left", "900");
    wrap("Enter the fresh four-digit PIN shown here before a transfer can begin.", 1035, 623, 220, 22, "#346c5c", "800 15px Trebuchet MS,Arial,sans-serif");

    var steps = ["Open the local portal on your computer.", "Select your own installed Steam copy of Balatro.", "Verify, transfer locally, then play with the TV remote."];
    for (var s = 0; s < steps.length; s++) {
      var sy = 724 + s * 44;
      ctx.fillStyle = "#ad3e4c"; ctx.beginPath(); ctx.arc(772, sy - 7, 16, 0, Math.PI * 2); ctx.fill();
      text(String(s + 1), 772, sy - 1, 15, "#fff8d9", "center");
      text(steps[s], 805, sy, 17, "#e9fbf3");
    }
    small("PREVIEW ONLY — PORTAL, PIN, VALIDATION, AND TRANSFER ARE NOT YET IMPLEMENTED", 755, 860, "#9bd7be");

    for (var p = particles.length - 1; p >= 0; p--) {
      var part = particles[p]; part.age += delta; part.x += part.vx * delta; part.y += part.vy * delta; part.vy += 500 * delta;
      if (part.age > .9) { particles.splice(p, 1); continue; }
      ctx.globalAlpha = 1 - part.age / .9; ctx.fillStyle = part.color; ctx.fillRect(part.x, part.y, 7, 7); ctx.globalAlpha = 1;
    }
    var tip = null;
    for (var a = 0; a < actions.length; a++) if (hit(actions[a].x, actions[a].y, actions[a].w, actions[a].h)) tip = actions[a].label;
    if (tip) drawTooltip(tip);
  }

  function makeGL() {
    var gl = canvas.getContext("webgl", { antialias: false, premultipliedAlpha: false });
    if (!gl) return null;
    var vs = "attribute vec2 p;varying vec2 v;void main(){v=(p+1.0)*.5;gl_Position=vec4(p,0.,1.);}";
    var fs = "precision mediump float;varying vec2 v;uniform sampler2D u;uniform float time;uniform float shake;"
      + "void main(){vec2 q=v-.5;float r=dot(q,q);vec2 uv=v+vec2(sin(time*55.)*.0007*shake,cos(time*47.)*.0007*shake);"
      + "float wave=sin(uv.x*13.+time*.25)+sin(uv.y*11.-time*.18)+sin((uv.x+uv.y)*9.+time*.11);vec3 felt=vec3(.025,.12,.08)+vec3(.02,.12,.07)*wave;"
      + "float edge=smoothstep(.2,.7,length(q));vec4 a=texture2D(u,uv);vec3 c=mix(felt,a.rgb,a.a);c+=vec3(edge*.012,0.,edge*.02)*(1.-a.a);"
      + "float scan=.92+.08*sin(gl_FragCoord.y*3.14159);c*=scan;float vig=1.-smoothstep(.18,.76,r)*.62;c*=vig;gl_FragColor=vec4(c,1.);}";
    function shader(type, source) { var sh = gl.createShader(type); gl.shaderSource(sh, source); gl.compileShader(sh); return sh; }
    var program = gl.createProgram(); gl.attachShader(program, shader(gl.VERTEX_SHADER, vs)); gl.attachShader(program, shader(gl.FRAGMENT_SHADER, fs)); gl.linkProgram(program);
    var buffer = gl.createBuffer(); gl.bindBuffer(gl.ARRAY_BUFFER, buffer); gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1,-1,1,-1,-1,1,1,1]), gl.STATIC_DRAW);
    var texture = gl.createTexture(); gl.bindTexture(gl.TEXTURE_2D, texture); gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR); gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR); gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE); gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    return { gl: gl, program: program, texture: texture, buffer: buffer, p: gl.getAttribLocation(program, "p"), time: gl.getUniformLocation(program, "time"), shake: gl.getUniformLocation(program, "shake"), u: gl.getUniformLocation(program, "u") };
  }

  var gpu = makeGL();
  var fallback = gpu ? null : canvas.getContext("2d", { alpha: false });
  function paint(now) {
    var t = (now - start) / 1000;
    var delta = Math.min(.05, Math.max(0, (now - lastFrame) / 1000));
    lastFrame = now;
    flash = Math.max(0, flash - delta * 1.5); shake = Math.max(0, shake - delta * 2.7);
    var textureDirty = !lastUiPaint || now - lastUiPaint >= 1000 / 30;
    if (textureDirty) {
      drawUI(t, lastUiPaint ? Math.min(.1, (now - lastUiPaint) / 1000) : 0);
      lastUiPaint = now;
    }
    if (gpu) {
      var gl = gpu.gl;
      if (canvas.width !== W || canvas.height !== H) { canvas.width = W; canvas.height = H; }
      gl.viewport(0, 0, canvas.width, canvas.height); gl.useProgram(gpu.program);
      gl.bindBuffer(gl.ARRAY_BUFFER, gpu.buffer); gl.enableVertexAttribArray(gpu.p); gl.vertexAttribPointer(gpu.p, 2, gl.FLOAT, false, 0, 0);
      gl.activeTexture(gl.TEXTURE0); gl.bindTexture(gl.TEXTURE_2D, gpu.texture); gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
      if (textureDirty) gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, ui);
      gl.uniform1i(gpu.u, 0); gl.uniform1f(gpu.time, t); gl.uniform1f(gpu.shake, shake); gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
    } else if (fallback && textureDirty) {
      fallback.drawImage(ui, 0, 0, W, H);
    }
    requestAnimationFrame(paint);
  }

  function toVirtual(event) { var b = canvas.getBoundingClientRect(); pointer.x = (event.clientX - b.left) * W / b.width; pointer.y = (event.clientY - b.top) * H / b.height; }
  canvas.addEventListener("pointermove", function (event) { toVirtual(event); });
  canvas.addEventListener("pointerleave", function () { pointer.x = -999; pointer.y = -999; });
  canvas.addEventListener("pointerdown", function (event) { toVirtual(event); pointer.down = true; flash = 1; shake = .45; addParticles(); });
  requestAnimationFrame(paint);
}());
