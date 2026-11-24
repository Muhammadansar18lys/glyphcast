// Glyphcast browser implementation. Mirrors the C++ and Python renderers:
// same box-averaging, Rec. 709 luminance, cell aspect and character ramps.

const RAMPS = {
  classic: " .:-=+*#%@",
  minimal: " .:#",
  dense: "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ",
};
const CELL_ASPECT = 0.5;

const els = {
  file: document.getElementById("file"),
  dropzone: document.getElementById("dropzone"),
  status: document.getElementById("status"),
  width: document.getElementById("width"),
  widthValue: document.getElementById("widthValue"),
  ramp: document.getElementById("ramp"),
  invert: document.getElementById("invert"),
  color: document.getElementById("color"),
  output: document.getElementById("output"),
  copy: document.getElementById("copy"),
  download: document.getElementById("download"),
};

let source = null;      // { width, height, data: Uint8ClampedArray }
let plainText = "";

function luminance(r, g, b) {
  return (0.2126 * r + 0.7152 * g + 0.0722 * b) / 255;
}

function escapeHtml(ch) {
  return ch.replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" })[c]);
}

function averageBlock(x0, y0, x1, y1) {
  const { width, data } = source;
  let r = 0, g = 0, b = 0, n = 0;
  for (let y = y0; y < y1; y++) {
    for (let x = x0; x < x1; x++) {
      const i = (y * width + x) * 4;
      r += data[i]; g += data[i + 1]; b += data[i + 2];
      n++;
    }
  }
  return [r / n, g / n, b / n];
}

function render() {
  els.widthValue.textContent = els.width.value;
  if (!source) return;

  const cols = Number(els.width.value);
  const rows = Math.max(1, Math.round((source.height / source.width) * cols * CELL_ASPECT));
  const ramp = RAMPS[els.ramp.value];
  const invert = els.invert.checked;
  const colour = els.color.checked;

  const plainLines = [];
  const htmlLines = [];

  for (let row = 0; row < rows; row++) {
    const y0 = Math.floor((row * source.height) / rows);
    const y1 = Math.max(y0 + 1, Math.floor(((row + 1) * source.height) / rows));
    let text = "";
    let html = "";

    for (let col = 0; col < cols; col++) {
      const x0 = Math.floor((col * source.width) / cols);
      const x1 = Math.max(x0 + 1, Math.floor(((col + 1) * source.width) / cols));
      const [r, g, b] = averageBlock(x0, y0, x1, y1);

      let lum = luminance(r, g, b);
      if (invert) lum = 1 - lum;
      const ch = ramp[Math.min(ramp.length - 1, Math.floor(lum * ramp.length))];

      text += ch;
      html += colour
        ? `<span style="color:rgb(${Math.trunc(r)},${Math.trunc(g)},${Math.trunc(b)})">${escapeHtml(ch)}</span>`
        : escapeHtml(ch);
    }
    plainLines.push(text);
    htmlLines.push(html);
  }

  plainText = plainLines.join("\n");
  els.output.innerHTML = htmlLines.join("\n");
}

function setStatus(message) {
  els.status.textContent = message;
}

function loadFile(file) {
  if (!file || !file.type.startsWith("image/")) {
    setStatus("Please choose an image file.");
    return;
  }
  const img = new Image();
  const url = URL.createObjectURL(file);
  img.onload = () => {
    const canvas = document.createElement("canvas");
    canvas.width = img.naturalWidth;
    canvas.height = img.naturalHeight;
    const ctx = canvas.getContext("2d");
    ctx.drawImage(img, 0, 0);
    source = {
      width: canvas.width,
      height: canvas.height,
      data: ctx.getImageData(0, 0, canvas.width, canvas.height).data,
    };
    URL.revokeObjectURL(url);
    setStatus(`${file.name} · ${canvas.width} × ${canvas.height}px`);
    render();
  };
  img.onerror = () => {
    URL.revokeObjectURL(url);
    setStatus("Could not read that image.");
  };
  img.src = url;
}

els.file.addEventListener("change", (event) => loadFile(event.target.files[0]));

["dragenter", "dragover"].forEach((type) =>
  els.dropzone.addEventListener(type, (event) => {
    event.preventDefault();
    els.dropzone.classList.add("dragover");
  })
);
["dragleave", "drop"].forEach((type) =>
  els.dropzone.addEventListener(type, (event) => {
    event.preventDefault();
    els.dropzone.classList.remove("dragover");
  })
);
els.dropzone.addEventListener("drop", (event) => loadFile(event.dataTransfer.files[0]));

[els.width, els.ramp, els.invert, els.color].forEach((control) =>
  control.addEventListener("input", render)
);

els.copy.addEventListener("click", async () => {
  if (!plainText) return;
  try {
    await navigator.clipboard.writeText(plainText);
    els.copy.textContent = "Copied!";
  } catch {
    els.copy.textContent = "Copy failed";
  }
  setTimeout(() => (els.copy.textContent = "Copy text"), 1500);
});

els.download.addEventListener("click", () => {
  if (!plainText) return;
  const blob = new Blob([plainText + "\n"], { type: "text/plain;charset=utf-8" });
  const link = document.createElement("a");
  link.href = URL.createObjectURL(blob);
  link.download = "glyphcast.txt";
  link.click();
  URL.revokeObjectURL(link.href);
});

render();
