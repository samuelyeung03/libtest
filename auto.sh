# Directory containing input video files
INPUT_DIR="./input"

# Iterate over each video file in the input directory and its subdirectories
find "$INPUT_DIR" -type f \( -name "*.mp4" -o -name "*.yuv" \) | while read -r video_file; do
  if [[ -f "$video_file" ]]; then
    # Extract resolution from the folder name
    resolution=$(basename "$(dirname "$video_file")")
    width=${resolution%x*}
    height=${resolution#*x}

    # Determine the input file to use
    if [[ "$video_file" == *.mp4 ]]; then
      yuv_file="${video_file%.mp4}.yuv"
      if [[ ! -f "$yuv_file" ]]; then
        echo "Converting $video_file to $yuv_file using ffmpeg with resolution ${width}x${height}..."
        ffmpeg -i "$video_file" -pix_fmt yuv420p -s "${width}x${height}" "$yuv_file"
      fi
    else
      yuv_file="$video_file"
    fi

    # Run the text_libx264 program with different options
    ./text_libx264 --i "$yuv_file" --dace 1 --runloops 1 --width "$width" --height "$height" --fps 30
    ./text_libx264 --i "$yuv_file" --dace 1 --runloops 10 --width "$width" --height "$height" --fps 30
    ./text_libx264 --i "$yuv_file" --dace 0 --runloops 1 --width "$width" --height "$height" --fps 30
    ./text_libx264 --i "$yuv_file" --dace 0 --runloops 10 --width "$width" --height "$height" --fps 30

    ./text_libx264 --i "$yuv_file" --dace 1 --runloops 1 --width "$width" --height "$height" --fps 60
    ./text_libx264 --i "$yuv_file" --dace 1 --runloops 10 --width "$width" --height "$height" --fps 60
    ./text_libx264 --i "$yuv_file" --dace 0 --runloops 1 --width "$width" --height "$height" --fps 60
    ./text_libx264 --i "$yuv_file" --dace 0 --runloops 10 --width "$width" --height "$height" --fps 60
  fi
done

python3 plotdata.py