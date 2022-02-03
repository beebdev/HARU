
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="File sanitiser script")
    parser.add_argument('-t', '--type', type=str, default='.paf', action='store',
                        help='File type to sanitise')
    parser.add_argument('-f', '--file', metavar='FILE', required=True,
                        dest='file_path', type=str, default=None, action='store',
                        help='File path to sanitise')
    parser.add_argument('-o', '--output', metavar='FILE',
                        dest='out_file', type=str, default=None, action='store',
                        help='File output path')
    args = parser.parse_args()

    with open(args.file_path, 'r') as f:
        if args.type == '.paf':
            lines = f.readlines()
            # clean up empty lines
            lines = [line.rstrip('\n') for line in lines]
            lines = [line for line in lines if line]
            # fix lines without newline
            for line in lines:
                cells = line.split('\t')
                if len(cells) > 12:
                    extra_line = cells[11][2:] + '\t'
                    extra_line += '\t'.join(cells[12:])
                    # lines.append(extra_line)
                    print(extra_line)
            
            # print to file
            if args.out_file:
                with open(args.out_file, 'w') as out_f:
                    for line in lines:
                        out_f.write(line + '\n')
            else:
                with open(args.file_path, 'w') as f:
                    for line in lines:
                        f.write(line + '\n')
