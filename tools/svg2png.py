"""Batch-convert SVG icons to transparent PNGs with headless Chrome/Edge.

Icons are white by default so the app can tint them at draw time.

Examples:
  python tools/svg2png.py icon.svg res/icons --sizes 16,20,24,32,40,48 --scale 4
  python tools/svg2png.py icon.svg res/icons24 --sizes 24 --scale 4
  python tools/svg2png.py icon.svg res/colored --sizes 24 --colors 409EFF,F56C6C
"""

import argparse
import math
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from PIL import Image


BROWSERS = [
    os.environ.get("CHROME_PATH", ""),
    r"C:\Program Files\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
]


def find_browser(explicit):
    if explicit:
        if Path(explicit).is_file():
            return explicit
        sys.exit(f"browser not found: {explicit}")
    for candidate in BROWSERS:
        if candidate and Path(candidate).is_file():
            return candidate
    for name in ("chrome", "chrome.exe", "msedge", "msedge.exe"):
        found = shutil.which(name)
        if found:
            return found
    sys.exit("Chrome/Edge not found; pass --browser PATH")


def parse_colors(value):
    colors = []
    for item in value.split(","):
        item = item.strip().lstrip("#")
        if len(item) != 6:
            raise argparse.ArgumentTypeError(f"invalid color: {item}")
        int(item, 16)
        colors.append(item.upper())
    if not colors:
        raise argparse.ArgumentTypeError("empty color list")
    return colors


def parse_sizes(value):
    sizes = [int(item) for item in value.split(",") if item.strip()]
    if not sizes or any(size <= 0 for size in sizes):
        raise argparse.ArgumentTypeError("invalid sizes")
    return sizes


def colorize(svg, color):
    if "currentColor" not in svg:
        raise ValueError("SVG has no currentColor to replace")
    return svg.replace("currentColor", f"#{color}")


def render_batch(browser, batch, render_size, columns, work_dir):
    rows = max(1, math.ceil(len(batch) / columns))
    width = columns * render_size
    height = rows * render_size
    cells = []
    for index, (svg, _color) in enumerate(batch):
        left = (index % columns) * render_size
        top = (index // columns) * render_size
        cells.append(
            f'<div class="cell" style="left:{left}px;top:{top}px;'
            f'width:{render_size}px;height:{render_size}px">{svg}</div>'
        )
    page = (
        "<!doctype html><html><head><meta charset='utf-8'><style>"
        "html,body{margin:0;padding:0;background:transparent;overflow:hidden}"
        ".cell{position:absolute;overflow:hidden}"
        ".cell>svg{display:block;width:100%;height:100%}"
        "</style></head><body>"
        + "".join(cells)
        + "</body></html>"
    )
    html_path = work_dir / "batch.html"
    png_path = work_dir / "batch.png"
    html_path.write_text(page, encoding="utf-8")
    command = [
        browser,
        "--headless=new",
        "--disable-gpu",
        "--hide-scrollbars",
        "--run-all-compositor-stages-before-draw",
        "--virtual-time-budget=1000",
        "--default-background-color=00000000",
        "--force-device-scale-factor=1",
        f"--window-size={width},{height}",
        f"--screenshot={png_path}",
        html_path.as_uri(),
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0 or not png_path.is_file():
        raise RuntimeError(result.stderr.strip() or "headless screenshot failed")
    image = Image.open(png_path).convert("RGBA")
    if image.size != (width, height):
        raise RuntimeError(f"unexpected screenshot size {image.size}, want {(width, height)}")
    return image


def main():
    parser = argparse.ArgumentParser(description="Convert SVG icons to colored PNGs")
    parser.add_argument("source", help="SVG file or directory")
    parser.add_argument("output", help="output directory")
    parser.add_argument("--sizes", default="24", help="comma-separated display sizes")
    parser.add_argument("--scale", type=int, default=4, help="PNG pixels per display px")
    parser.add_argument("--colors", default="FFFFFF", help="comma-separated RRGGBB colors")
    parser.add_argument("--columns", type=int, default=16, help="icons per render row")
    parser.add_argument("--batch", type=int, default=256, help="icons per screenshot")
    parser.add_argument("--browser", default="", help="Chrome/Edge executable")
    parser.add_argument("--skip-existing", action="store_true", help="keep existing PNGs")
    args = parser.parse_args()

    source = Path(args.source)
    if source.is_dir():
        files = sorted(source.glob("*.svg"))
    elif source.is_file():
        files = [source]
    else:
        sys.exit(f"source not found: {source}")
    if not files:
        sys.exit(f"no SVG files in {source}")

    sizes = parse_sizes(args.sizes)
    colors = parse_colors(args.colors)
    browser = find_browser(args.browser)
    output = Path(args.output)
    output.mkdir(parents=True, exist_ok=True)

    work = Path(tempfile.mkdtemp(prefix="svg2png-"))
    total = 0
    try:
        for size in sizes:
            render_size = size * args.scale
            targets = []
            for index, path in enumerate(files):
                name = f"{path.stem}-{size}.png" if len(sizes) > 1 else f"{path.stem}.png"
                target = output / name
                if args.skip_existing and target.is_file():
                    continue
                svg = colorize(path.read_text(encoding="utf-8"), colors[index % len(colors)])
                targets.append((path, target, svg))
            if not targets:
                continue

            for start in range(0, len(targets), args.batch):
                chunk = targets[start:start + args.batch]
                batch = [(item[2], "") for item in chunk]
                image = render_batch(browser, batch, render_size, args.columns, work)
                for index, (_path, target, _svg) in enumerate(chunk):
                    left = (index % args.columns) * render_size
                    top = (index // args.columns) * render_size
                    icon = image.crop((left, top, left + render_size, top + render_size))
                    icon.save(target, optimize=True)
                total += len(chunk)
                print(f"[{size}px] {start + len(chunk)}/{len(targets)}")
        print(f"done: {total} PNGs -> {output}")
    finally:
        shutil.rmtree(work, ignore_errors=True)


if __name__ == "__main__":
    main()
