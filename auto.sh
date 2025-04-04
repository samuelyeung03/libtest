# Directory containing input video files
INPUT_DIR="./input"
BUILD_DIR="./out"
OUTPUT_DIR="result/"

# Default value for runloops
RUNLOOPS=1

# Default value for fps
FPS_VALUES=("30") # Default FPS value

# Default value for bitrate
BITRATE_VALUES=("3000") # Default bitrate value

FORCE=0
NO_RECORD=0

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
  case $1 in
    --r) RUNLOOPS="$2"; shift ;; # Set the runloops value
    --fixed) MODE="--fixed" ;; # Set the mode to --fixed
    --c) COMPLEXITY="$2"; shift ;; # Set the complexity value
    --all) MODE="--all" ;; # Set the mode to --all
    --B) BUILD="$2"; shift ;;
    --fps) FPS_VALUES+=("$2"); shift ;; # Add FPS value to the list
    --br) BITRATE_VALUES+=("$2"); shift ;; # Add bitrate value to the list
    --force) FORCE=1 ;; # Set the force flag
    --no_record) NO_RECORD=1 ;; # Set the no_record flag
    *) echo "Unknown option: $1"; exit 1 ;;
  esac
  shift
done

# Remove default FPS if user specifies custom values
if [[ "${#FPS_VALUES[@]}" -gt 1 ]]; then
  FPS_VALUES=("${FPS_VALUES[@]:1}")
fi

# Remove default bitrate if user specifies custom values
if [[ "${#BITRATE_VALUES[@]}" -gt 1 ]]; then
  BITRATE_VALUES=("${BITRATE_VALUES[@]:1}")
fi

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

# Determine the test_libx264 binary/binaries to use
if [[ -n "$BUILD" ]]; then
  TEST_BINARIES=("$BUILD_DIR/$BUILD/test_libx264")
else
  TEST_BINARIES=($(find "$BUILD_DIR" -type f -name "test_libx264"))
fi

# Process .yuv files
find "$INPUT_DIR" -type f -name "*.yuv" | while read -r yuv_file; do
  if [[ -f "$yuv_file" ]]; then
    # Extract resolution from the folder name
    resolution=$(basename "$(dirname "$yuv_file")")
    width=${resolution%x*}
    height=${resolution#*x}
    out_dir="$OUTPUT_DIR$(basename "${yuv_file%.yuv}")" # Fixed syntax error
    echo "OUTPUT_DIR: $out_dir" # Print the OUTPUT_DIR value
    for test_binary in "${TEST_BINARIES[@]}"; do
      build_name=$(basename "$(dirname "$test_binary")") # Extract the parent folder name
      if [[ "$MODE" == "--fixed" ]]; then
        if [[ -n "$COMPLEXITY" ]]; then
          for bitrate in "${BITRATE_VALUES[@]}"; do
            target_file="${out_dir}/build:${build_name}_complexity:${COMPLEXITY}_bitrate:${bitrate}_runloops:${RUNLOOPS}.json"
            if [[ -f "$target_file" && $FORCE -ne 1 ]]; then
              echo "Skipping existing file: $target_file"
              continue
            fi
            "$test_binary" --i "$yuv_file" --dace 1 --c "$COMPLEXITY" --runloops "$RUNLOOPS" --width "$width" --height "$height" --fps 30 --br "$bitrate" --length 900 --dir "$out_dir" --name "build:${build_name}_complexity:${COMPLEXITY}_bitrate:${bitrate}_runloops:${RUNLOOPS}"
            if [[ $NO_RECORD -eq 1 ]]; then
              rm -f "${out_dir}/build:${build_name}_complexity:${COMPLEXITY}_bitrate:${bitrate}_runloops:${RUNLOOPS}.h264"
            fi
          done
        else
          for i in {0..9}; do
            for bitrate in "${BITRATE_VALUES[@]}"; do
              target_file="${out_dir}/build:${build_name}_complexity:${i}_bitrate:${bitrate}_runloops:${RUNLOOPS}.json"
              if [[ -f "$target_file" && $FORCE -ne 1 ]]; then
                echo "Skipping existing file: $target_file"
                continue
              fi
              "$test_binary" --i "$yuv_file" --dace 1 --c "$i" --runloops "$RUNLOOPS" --width "$width" --height "$height" --fps 30 --br "$bitrate" --length 900 --dir "$out_dir" --name "build:${build_name}_complexity:${i}_bitrate:${bitrate}_runloops:${RUNLOOPS}"
              if [[ $NO_RECORD -eq 1 ]]; then
                rm -f "${out_dir}/build:${build_name}_complexity:${i}_bitrate:${bitrate}_runloops:${RUNLOOPS}.h264"
              fi
            done
          done
        fi
      fi
      if [[ "$MODE" != "--fixed" || "$MODE" == "--all" ]]; then
        for fps in "${FPS_VALUES[@]}"; do
          for bitrate in "${BITRATE_VALUES[@]}"; do
            target_file="${out_dir}/build:${build_name}_fps:${fps}_bitrate:${bitrate}_runloops:${RUNLOOPS}_dace:1.json"
            if [[ -f "$target_file" && $FORCE -ne 1 ]]; then
              echo "Skipping existing file: $target_file"
            else
              "$test_binary" --i "$yuv_file" --dace 1 --runloops "$RUNLOOPS" --width "$width" --height "$height" --fps "$fps" --br "$bitrate" --length 900 --dir "$out_dir" --name "build:${build_name}_fps:${fps}_bitrate:${bitrate}_runloops:${RUNLOOPS}_dace:1"
              if [[ $NO_RECORD -eq 1 ]]; then
                rm -f "${out_dir}/build:${build_name}_fps:${fps}_bitrate:${bitrate}_runloops:${RUNLOOPS}_dace:1.h264"
              fi
            fi
            target_file="${out_dir}/build:${build_name}_fps:${fps}_bitrate:${bitrate}_runloops:${RUNLOOPS}_dace:0.json"
            if [[ -f "$target_file" && $FORCE -ne 1 ]]; then
              echo "Skipping existing file: $target_file"
            else
              "$test_binary" --i "$yuv_file" --dace 0 --runloops "$RUNLOOPS" --width "$width" --height "$height" --fps "$fps" --br "$bitrate" --length 900 --dir "$out_dir" --name "build:${build_name}_fps:${fps}_bitrate:${bitrate}_runloops:${RUNLOOPS}_dace:0"
            fi
            if [[ $NO_RECORD -eq 1 ]]; then
              rm -f "${out_dir}/build:${build_name}_fps:${fps}_bitrate:${bitrate}_runloops:${RUNLOOPS}_dace:0.h264"
            fi
          done
        done
      fi
    done
  fi
done

python3 plotdata.py