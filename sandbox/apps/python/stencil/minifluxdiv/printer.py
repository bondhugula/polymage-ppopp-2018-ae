import sys

def print_line(to_file=None):
    if to_file:
        print("--------------------------------------------------", file=to_file)
    else:
        print("--------------------------------------------------")

def print_header():
    print("MiniFluxDiv")

def print_usage():
    print("[main]: Usage: ")
    print("[main]: "+sys.argv[0]+" <mode> <image> ", end=" ")
    print("<rows> <cols> <nruns>")
    print("[main]: <mode>  :: {'new', 'existing', 'tune'}")

def print_config(app_data):
    app_args = app_data['app_args']
    print_line()
    print("# Problem Settings #")
    print("")
    print("[main]: mode        =", app_args.mode)
    print("[main]: image       =", app_args.img_file)
    print("[main]: sim size    = %d x %d" % (int(app_args.boxes), int(app_args.cells)))
    print("[main]: nruns       =", app_args.runs)
    print_line()
