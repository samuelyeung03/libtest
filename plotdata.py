import json
import os
import matplotlib.pyplot as plt
import math

result_folder = './result'

os.makedirs(result_folder, exist_ok=True)

for subfolder in os.listdir(result_folder):
    subfolder_path = os.path.join(result_folder, subfolder)
    graphs_folder = os.path.join(subfolder_path, 'graphs')
    os.makedirs(graphs_folder, exist_ok=True)

    # Initialize a dictionary to store aggregated data for all keys
    aggregated_data = {}

    # Iterate through all JSON files in the result folder and its subfolders
    for filename in os.listdir(subfolder_path):
        if filename.endswith('.json'):
            file_path = os.path.join(subfolder_path, filename)
            
            # Load the JSON data
            with open(file_path, 'r') as file:
                data = json.load(file)
            
            # Dynamically process all keys in the JSON file
            for key, values in data.items():
                if isinstance(values, list):  # Only process list-type values
                    if key not in aggregated_data:
                        aggregated_data[key] = []
                    aggregated_data[key].append((filename, values))

    # Plot the aggregated data and save the main plot
    plt.figure(figsize=(12, 6))

    # Dynamically plot data for each key
    for key, datasets in aggregated_data.items():
        for filename, values in datasets:
            plt.plot(values, label=f'{key} ({filename})', alpha=0.7)

    # Add labels, title, and legend
    plt.xlabel('Index')
    plt.ylabel('Values')
    plt.title('Aggregated Graph of JSON Data')
    plt.legend(loc='upper right')  # Set legend position

    # Save the main aggregated plot without showing it
    plt.tight_layout()
    plt.savefig(os.path.join(graphs_folder, 'aggregated_graph.png'))
    plt.close()  # Close the plot instead of showing it

    # Plot each key in a separate plot and save them
    for key, datasets in aggregated_data.items():
        plt.figure(figsize=(10, 5))
        for filename, values in datasets:
            plt.plot(values, label=f'{filename}', alpha=0.7)
        plt.xlabel('Index')
        plt.ylabel('Values')
        plt.title(f'{key}')
        plt.legend(loc='upper right')  # Set legend position
        plt.tight_layout()
        plt.savefig(os.path.join(graphs_folder, f'{key}_graph.png'))
        plt.close()  # Close the plot instead of showing it

    # Ensure the "analysis" file is created in the result folder
    analysis_file_path = os.path.join(subfolder_path, 'analysis.txt')
    with open(analysis_file_path, 'w') as analysis_file:
        # Calculate and write the average of each key for each file
        for key, datasets in aggregated_data.items():
            analysis_file.write(f'\nAverages for key: {key}\n')
            for filename, values in datasets:
                file_average = sum(values) / len(values) if values else 0
                analysis_file.write(f'  Average for {filename}: {file_average}\n')

        # Calculate SSIM in dB if the key 'ssim' exists
        if 'ssim' in aggregated_data:
            analysis_file.write('\nSSIM in dB:\n')
            for filename, values in aggregated_data['ssim']:
                ssim_db_values = [-10.0 * math.log10(1 - value) for value in values if 0 < value < 1]
                avg_ssim_db = sum(ssim_db_values) / len(ssim_db_values) if ssim_db_values else float('-inf')
                analysis_file.write(f'  Average SSIM in dB for {filename}: {avg_ssim_db}\n')

        # Calculate and write percentage difference adn peak difference in SSIM and PSNR between all file pairs
        from itertools import combinations
        keys_to_compare = ['ssim', 'psnr']  # Keys to compare
        for key in keys_to_compare:
            if key in aggregated_data and len(aggregated_data[key]) > 1:
                for (file1, values1), (file2, values2) in combinations(aggregated_data[key], 2):
                    if len(values1) == len(values2):  # Ensure both files have the same number of values
                        percentage_diff = [
                            abs(v1 - v2) / ((v1 + v2) / 2) * 100 if (v1 + v2) != 0 else 0
                            for v1, v2 in zip(values1, values2)
                        ]
                        avg_diff = sum(percentage_diff) / len(percentage_diff)
                        
                        top_10_percent = sorted(percentage_diff, reverse=True)[:max(1, len(percentage_diff) // 10)]
                        avg_peak_10_diff = sum(top_10_percent) / len(top_10_percent)
                        top_5_percent = sorted(percentage_diff, reverse=True)[:max(1, len(percentage_diff) // 20)]
                        avg_peak_5_diff = sum(top_5_percent) / len(top_5_percent)
                        top_1_percent = sorted(percentage_diff, reverse=True)[:max(1, len(percentage_diff) // 100)]
                        avg_peak_1_diff = sum(top_1_percent) / len(top_1_percent)
                        
                        analysis_file.write(f'\nPercentage difference in {key} between {file1} and {file2}\n')
                        analysis_file.write(f'  Average: {avg_diff}%\n')
                        analysis_file.write(f'  Peak 10%: {avg_peak_10_diff}%\n')
                        analysis_file.write(f'  Peak 5%: {avg_peak_5_diff}%\n')
                        analysis_file.write(f'  Peak 1%: {avg_peak_1_diff}%\n')

                        # Calculate absolute value differences
                        abs_diff = [abs(v1 - v2) for v1, v2 in zip(values1, values2)]
                        avg_abs_diff = sum(abs_diff) / len(abs_diff)

                        abs_top_10_percent = sorted(abs_diff, reverse=True)[:max(1, len(abs_diff) // 10)]
                        avg_abs_peak_10_diff = sum(abs_top_10_percent) / len(abs_top_10_percent)
                        abs_top_5_percent = sorted(abs_diff, reverse=True)[:max(1, len(abs_diff) // 20)]
                        avg_abs_peak_5_diff = sum(abs_top_5_percent) / len(abs_top_5_percent)
                        abs_top_1_percent = sorted(abs_diff, reverse=True)[:max(1, len(abs_diff) // 100)]
                        avg_abs_peak_1_diff = sum(abs_top_1_percent) / len(abs_top_1_percent)

                        analysis_file.write(f'  Absolute Difference Average: {avg_abs_diff}\n')
                        analysis_file.write(f'  Absolute Peak 10%: {avg_abs_peak_10_diff}\n')
                        analysis_file.write(f'  Absolute Peak 5%: {avg_abs_peak_5_diff}\n')
                        analysis_file.write(f'  Absolute Peak 1%: {avg_abs_peak_1_diff}\n')
                    else:
                        analysis_file.write(f'Cannot compare {key} between {file1} and {file2} due to unequal lengths.\n')
            else:
                analysis_file.write(f'Key {key} not found or does not have enough datasets for comparison.\n')

    # Calculate and print the average of each key for each file
    for key, datasets in aggregated_data.items():
        print(f'\nAverages for key: {key}')
        for filename, values in datasets:
            file_average = sum(values) / len(values) if values else 0
            print(f'  Average for {filename}: {file_average}')

    # Calculate and plot percentage difference in SSIM and PSNR between all file pairs
    from itertools import combinations

    keys_to_compare = ['ssim', 'psnr']  # Keys to compare
    for key in keys_to_compare:
        if key in aggregated_data and len(aggregated_data[key]) > 1:
            # Generate all pairwise combinations of files
            for (file1, values1), (file2, values2) in combinations(aggregated_data[key], 2):
                if len(values1) == len(values2):  # Ensure both files have the same number of values
                    percentage_diff = [
                        abs(v1 - v2) / ((v1 + v2) / 2) * 100 if (v1 + v2) != 0 else 0
                        for v1, v2 in zip(values1, values2)
                    ]
                    avg_diff = sum(percentage_diff) / len(percentage_diff)
                    print(f'Average percentage difference in {key} between {file1} and {file2}: {avg_diff}%')

                    # Plot percentage difference
                    plt.figure(figsize=(10, 5))
                    plt.plot(percentage_diff, label=f'Percentage Difference in {key}', alpha=0.7)
                    plt.xlabel('Index')
                    plt.ylabel('Percentage Difference (%)')
                    plt.title(f'Percentage Difference in {key}')
                    plt.legend(loc='upper right')  # Set legend position
                    # Save percentage difference plot without showing it
                    plt.tight_layout()
                    plt.savefig(os.path.join(graphs_folder, f'percentage_diff_{key}_{file1}_vs_{file2}.png'))
                    plt.close()  # Close the plot instead of showing it
                else:
                    print(f'Cannot compare {key} between {file1} and {file2} due to unequal lengths.')
        else:
            print(f'Key {key} not found or does not have enough datasets for comparison.')

    # Plot dace_complexity and durations in the same graph for each file if the keys are found
    if 'dace_complexity' in aggregated_data and 'durations' in aggregated_data:
        for (file1, complexities), (file2, durations) in zip(aggregated_data['dace_complexity'], aggregated_data['durations']):
            if file1 == file2:  # Ensure the data corresponds to the same file
                plt.figure(figsize=(10, 5))
                plt.plot(complexities, label='Dace Complexity', alpha=0.7)
                plt.plot(durations, label='Durations', alpha=0.7)
                plt.xlabel('Index')
                plt.ylabel('Values')
                plt.title(f'Dace Complexity and Durations for {file1}')
                plt.legend(loc='upper right')  # Set legend position
                plt.tight_layout()
                plt.savefig(os.path.join(graphs_folder, f'dace_complexity_and_durations_{file1}.png'))
                plt.close()  # Close the plot instead of showing it

    # Plot (33333 - durations) if the key 'durations' is found
    if 'durations' in aggregated_data:
        for filename, durations in aggregated_data['durations']:
            adjusted_values = [33333 - value for value in durations]
            plt.figure(figsize=(10, 5))
            plt.plot(adjusted_values, label='(33333 - Durations)', alpha=0.7)
            plt.xlabel('Index')
            plt.ylabel('Values')
            plt.title(f'(33333 - Durations) for {filename}')
            plt.legend(loc='upper right')  # Set legend position
            plt.tight_layout()
            plt.savefig(os.path.join(graphs_folder, f'33333_minus_durations_{filename}.png'))
            plt.close()  # Close the plot instead of showing it