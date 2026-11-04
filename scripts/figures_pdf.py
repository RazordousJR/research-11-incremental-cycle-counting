"""Export the six paper figures to vector PDF for JGAA with headless Microsoft Edge, and check them. Nothing is redrawn: run figures.py and figures_results.py first.
Usage: figures_pdf.py [scratch dir]
Each SVG prints at the JGAA text width (15 cm, 425 pt) or less, so a viewBox WIDTH units wide gives one point per unit. Checks per figure: viewBox at most 425 x 567,
every font size at least 10 pt as printed, the PDF a single page of the viewBox size (pdfinfo), and every font embedded (pdffonts). It also writes a PNG copy
beside each SVG at 2 px per point, from the PDF, for looking at only. The scratch dir holds the HTML wrappers and Edge's profile; by default a new temp dir."""
import os
import re
import subprocess
import sys
import tempfile

EDGE = r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
FIGS = ["fig-worked-batch", "fig-stream-timeline", "fig-test-set", "fig-score-by-p", "fig-cost-model", "fig-theta"]
TEXT_W, TEXT_H, MIN_PT = 425, 567, 10  # JGAA text block in points, smallest printed font


def run(*cmd):
    return subprocess.run(cmd, capture_output=True, text=True, timeout=180).stdout


def check_svg(src):
    w, h = map(float, re.search(r'viewBox="0 0 ([\d.]+) ([\d.]+)"', src).groups())
    printed = min(w, TEXT_W)  # printed width in points: the text width, or the viewBox if narrower
    small = min(float(f) for f in re.findall(r'font-size="([\d.]+)"', src)) * printed / w
    assert w <= TEXT_W and h * printed / w <= TEXT_H, f"{w} x {h} does not fit the text block"
    assert small >= MIN_PT, f"smallest font {small:.2f} pt"
    return w, h, printed, small


def export(name, scratch):
    src = open(f"docs/figures/{name}.svg", encoding="utf-8").read()
    w, h, printed, small = check_svg(src)
    ph = h * printed / w
    tag = re.sub(r'width="[\d.]+" height="[\d.]+"', f'width="{printed}pt" height="{ph}pt"', src.split("\n", 1)[0], count=1)
    html = (f"<!doctype html><html><head><meta charset='utf-8'><style>@page {{ size: {printed}pt {ph}pt; margin: 0 }} "
            f"html, body {{ margin: 0; padding: 0; overflow: hidden }} svg {{ display: block }}</style></head><body>\n{tag}\n{src.split(chr(10), 1)[1]}</body></html>\n")
    page = os.path.join(scratch, f"{name}.html")
    open(page, "w", encoding="utf-8", newline="\n").write(html)
    pdf = os.path.abspath(f"docs/figures/pdf/{name}.pdf")
    if os.path.exists(pdf):
        os.remove(pdf)
    run(EDGE, "--headless=new", "--disable-gpu", "--no-pdf-header-footer", f"--user-data-dir={os.path.join(scratch, 'edge')}",
        f"--print-to-pdf={pdf}", "file:///" + os.path.abspath(page).replace("\\", "/"))
    assert os.path.exists(pdf), f"Edge wrote no {pdf}"
    info = run("pdfinfo", pdf)
    pages = int(re.search(r"Pages:\s+(\d+)", info).group(1))
    pw, pg = map(float, re.search(r"Page size:\s+([\d.]+) x ([\d.]+) pts", info).groups())
    assert pages == 1 and abs(pw - printed) < 1 and abs(pg - ph) < 1, f"{pages} pages of {pw} x {pg} pts, wanted {printed} x {ph:.1f}"
    fonts = [ln.split() for ln in run("pdffonts", pdf).splitlines()[2:] if ln.strip()]
    assert all("yes" in f[-5:-4] for f in fonts), f"a font is not embedded: {fonts}"
    run("pdftoppm", "-png", "-r", "144", "-singlefile", pdf, f"docs/figures/{name}")
    return dict(viewBox=f"{w:g} x {h:g}", printed_pt=printed, min_font_pt=round(small, 2), page=f"{pw:g} x {pg:g} pts", fonts=sorted({f[0] for f in fonts}))


if __name__ == "__main__":
    scratch = sys.argv[1] if len(sys.argv) > 1 else tempfile.mkdtemp(prefix="figs-")
    os.makedirs(scratch, exist_ok=True)
    os.makedirs("docs/figures/pdf", exist_ok=True)
    for name in FIGS:
        print(name, export(name, scratch))
