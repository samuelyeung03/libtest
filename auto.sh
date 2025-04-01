# Directory containing input video files
INPUT_DIR="./input"

# Default value for runloops
RUNLOOPS=1

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
  case $1 in
    --r) RUNLOOPS="$2"; shift ;; # Set the runloops value
    --fixed) MODE="--fixed" ;;                   # Set the mode to --fixed
    *) echo "Unknown option: $1"; exit 1 ;;
  esac
  shift
done

# Process .mp4 files first
find "$INPUT_DIR" -type f -name "*.mp4" | while read -r video_file; do
  if [[ -f "$video_file" ]]; then
    # Extract resolution from the folder name
    resolution=$(basename "$(dirname "$video_file")")
    width=${resolution%x*}
    height=${resolution#*x}

    # Convert .mp4 to .yuv if necessary
    yuv_file="${video_file%.mp4}.yuv"
    if [[ ! -f "$yuv_file" ]]; then
      echo "Converting $video_file to $yuv_file using ffmpeg with resolution ${width}x${height}..."
      ffmpeg -i "$video_file" -pix_fmt yuv420p -s "${width}x${height}" -frames:v 900 "$yuv_file"
    fi
  fi
done

# Process .yuv files
find "$INPUT_DIR" -type f -name "*.yuv" | while read -r yuv_file; do
  if [[ -f "$yuv_file" ]]; then
    # Extract resolution from the folder name
    resolution=$(basename "$(dirname "$yuv_file")")
    width=${resolution%x*}
    height=${resolution#*x}

    if [[ "$MODE" == "--fixed" ]]; then
        for i in {1..10}; do
          ./test_libx264 --i "$yuv_file" --dace 1 --c i --runloops "$RUNLOOPS" --width "$width" --height "$height" --fps 30 --length 900
        done
    else
      # Run the test_libx264 program with different options
      ./test_libx264 --i "$yuv_file" --dace 1 --runloops 1 --width "$width" --height "$height" --fps 30 --length 900
      ./test_libx264 --i "$yuv_file" --dace 0 --runloops 1 --width "$width" --height "$height" --fps 30 --length 900
      ./test_libx264 --i "$yuv_file" --dace 1 --runloops "$RUNLOOPS" --width "$width" --height "$height" --fps 60 --length 900
      ./test_libx264 --i "$yuv_file" --dace 0 --runloops "$RUNLOOPS" --width "$width" --height "$height" --fps 60 --length 900
    fi
  fi
done

python3 plotdata.py