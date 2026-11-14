"""Extract exact production code for replay with external I/O test doubles."""
import argparse
from pathlib import Path
import subprocess

parser=argparse.ArgumentParser()
parser.add_argument('repo',type=Path)
parser.add_argument('revision')
parser.add_argument('dest',type=Path)
args=parser.parse_args()

def src(path):
    return subprocess.check_output(['git','-C',args.repo,'show',f'{args.revision}:{path}'],text=True)

def region(s,begin,end):
    i=s.index(begin)
    return s[i:s.index(end,i)]

main=src('src/main.cpp')
qt=src('server/armv5-unknown-linux-gnueabihf/Controller.cpp')
args.dest.mkdir(parents=True,exist_ok=True)
(args.dest/'production.hpp').write_text(src('src/resume_policy.hpp'))
(args.dest/'input.inc').write_text(region(main,'#define ACTION_RELEASE','static void cursor_position_callback'))
(args.dest/'qt.inc').write_text(region(qt,'void Controller::ObserveHandoff','void Controller::SetVolume'))
(args.dest/'loop.inc').write_text(region(main,'#ifndef DESKTOP\n    wampy_review::ResumePolicy resume;' if '    const bool fastResume' not in main else '#ifndef DESKTOP\n    const bool fastResume','        // Reload queued resources'))
