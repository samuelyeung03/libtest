# Directory containing input video files
INPUT_DIR="./input"

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

    # Run the text_libx264 program with different options
    ./test_libx264 --i "$yuv_file" --dace 1 --runloops 1 --width "$width" --height "$height" --fps 30 --length 900
    ./test_libx264 --i "$yuv_file" --dace 1 --runloops 5 --width "$width" --height "$height" --fps 30 --length 900
    ./test_libx264 --i "$yuv_file" --dace 0 --runloops 1 --width "$width" --height "$height" --fps 30 --length 900
    ./test_libx264 --i "$yuv_file" --dace 0 --runloops 5 --width "$width" --height "$height" --fps 30 --length 900

    ./test_libx264 --i "$yuv_file" --dace 1 --runloops 1 --width "$width" --height "$height" --fps 60 --length 900
    ./test_libx264 --i "$yuv_file" --dace 1 --runloops 5 --width "$width" --height "$height" --fps 60 --length 900
    ./test_libx264 --i "$yuv_file" --dace 0 --runloops 1 --width "$width" --height "$height" --fps 60 --length 900
    ./test_libx264 --i "$yuv_file" --dace 0 --runloops 5 --width "$width" --height "$height" --fps 60 --length 900
  fi
done

python3 plotdata.py