"""Extract unchanged production bodies; only external device/Qt I/O is doubled."""
from pathlib import Path
import sys
root, output = map(Path, sys.argv[1:])
output.mkdir(parents=True, exist_ok=True)
def section(path, start, end):
    text = (root / path).read_text()
    return text[text.index(start):text.index(end, text.index(start))]
pieces = {
    'input.inc': ('src/main.cpp', '#define ACTION_RELEASE', 'static void cursor_position_callback'),
    'qt.inc': ('server/armv5-unknown-linux-gnueabihf/Controller.cpp', 'void Controller::ObserveHandoff', 'void Controller::SetVolume'),
    'handoff.inc': ('src/connector/hagoromo.cpp', '    bool HagoromoConnector::WindowHidden', '    void HagoromoConnector::ApplyUserSelection'),
    'loop.inc': ('src/main.cpp', '#ifndef DESKTOP\n    graphicsFailure = false;', '        ImGui_ImplOpenGL3_NewFrame();'),
}
for name, args in pieces.items():
    (output / name).write_text(section(*args))
