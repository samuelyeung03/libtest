import json
import os
import matplotlib.pyplot as plt

# Define the folder containing the JSON files
result_folder = './result'

# Initialize a dictionary to store aggregated data for all keys
aggregated_data = {}

# Iterate through all JSON files in the folder
for filename in os.listdir(result_folder):
    if filename.endswith('.json'):
        file_path = os.path.join(result_folder, filename)
        
        # Load the JSON data
        with open(file_path, 'r') as file:
            data = json.load(file)
        
        # Dynamically process all keys in the JSON file
        for key, values in data.items():
            if isinstance(values, list):  # Only process list-type values
                if key not in aggregated_data:
                    aggregated_data[key] = []
                aggregated_data[key].append((filename, values))

# Plot the aggregated data
plt.figure(figsize=(12, 6))

# Dynamically plot data for each key
for key, datasets in aggregated_data.items():
    for filename, values in datasets:
        plt.plot(values, label=f'{key} ({filename})', alpha=0.7)

# Add labels, title, and legend
plt.xlabel('Index')
plt.ylabel('Values')
plt.title('Aggregated Graph of JSON Data')
plt.legend()

# Show the main aggregated plot
plt.tight_layout()
plt.show()

# Plot each key in a separate plot
for key, datasets in aggregated_data.items():
    plt.figure(figsize=(10, 5))
    for filename, values in datasets:
        plt.plot(values, label=f'{filename}', alpha=0.7)
    plt.xlabel('Index')
    plt.ylabel('Values')
    plt.title(f'Graph for {key}')
    plt.legend()
    plt.tight_layout()
    plt.show()