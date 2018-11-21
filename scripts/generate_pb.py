import fnmatch
import os
import argparse
import sys
from distutils.spawn import find_executable
import subprocess

def str2bool(v):
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

parser = argparse.ArgumentParser(description="pb gen")

parser.add_argument("--list-dependencies", type=str2bool, nargs='?',
                        const=True, default=False,
                        help="Output the list of protobuf files")
parser.add_argument("--generate", type=str2bool, nargs='?',
                        const=True, default=False,
                        help="Generate ./.cc file for input .proto files")

parser.add_argument("--include")
parser.add_argument("--output")
parser.add_argument("--recurse-start-dir")
parsed_args = parser.parse_args()

usage = 'usage: python generate_pb.py --list-dependencies --generate --include=<include_for_protoc> --output=<output_dir> --recurse-start-dir=<recurse_start_dir>'
matches = []
 
for root, dirnames, filenames in os.walk(parsed_args.recurse_start_dir):
    for filename in fnmatch.filter(filenames, '*.proto'):
        matches.append(os.path.join(root, filename))


print(matches)
sys.stdout.write(str(parsed_args))
if parsed_args.list_dependencies:
    sys.stdout.write(';'.join(matches))

if parsed_args.generate:
    protoc = find_executable('protoc')
    for f in matches:
        #cmd = protoc + ' --cpp_out=' + parsed_args.output + ' ' + parsed_args.include + ' ' + f
        cmd = protoc + ' --validate_out="lang=cc:"' + parsed_args.output + ' --cpp_out=' + parsed_args.output + ' ' + parsed_args.include + ' ' + f
        print cmd
	subprocess.call(cmd, shell=True)
