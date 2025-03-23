import os
import json
import matplotlib.pyplot as plt

# Function to load JSON data from a file
def load_json(file_path):
    with open(file_path, 'r') as f:
        return json.load(f)
# Function to plot and compare all parameter sets
def plot_all_comparisons(data_list, labels, title):
    # Plot SSIM comparison
    plt.figure(figsize=(12, 8))
    for data, label in zip(data_list, labels):
        plt.plot(data['ssim'], label=f'{label}_ssim', alpha=0.7)
    plt.xlabel('Frame')
    plt.ylabel('SSIM')
    plt.title(f"{title} - SSIM")
    plt.legend()
    plt.grid(True)
    all_ssim_values = [value for data in data_list for value in data['ssim']]
    plt.ylim(min(all_ssim_values) * 0.95, max(all_ssim_values) * 1.05)
    plt.show()

    # Plot PSNR comparison
    plt.figure(figsize=(12, 8))
    for data, label in zip(data_list, labels):
        plt.plot(data['psnr'], label=f'{label}_psnr', alpha=0.7)
    plt.xlabel('Frame')
    plt.ylabel('PSNR')
    plt.title(f"{title} - PSNR")
    plt.legend()
    plt.grid(True)
    all_psnr_values = [value for data in data_list for value in data['psnr']]
    plt.ylim(min(all_psnr_values) * 0.95, max(all_psnr_values) * 1.05)
    plt.show()

    # Plot Duration comparison
    plt.figure(figsize=(12, 8))
    for data, label in zip(data_list, labels):
        plt.plot(data['duration'], label=f'{label}_duration', alpha=0.7)
    plt.xlabel('Frame')
    plt.ylabel('Duration (ms)')
    plt.title(f"{title} - Duration")
    plt.legend()
    plt.grid(True)
    all_duration_values = [value for data in data_list for value in data['duration']]
    plt.ylim(min(all_duration_values) * 0.95, max(all_duration_values) * 1.05)
    plt.show()
# # Function to plot and compare data for two selected parameters
# def plot_comparison(data1, data2, label1, label2, title):
#     # Plot SSIM comparison
#     plt.figure(figsize=(12, 8))
#     plt.plot(data1['ssim'], label=f'{label1}_ssim', alpha=0.7, color='green')
#     plt.plot(data2['ssim'], label=f'{label2}_ssim', alpha=0.7, color='purple')
#     plt.xlabel('Frame')
#     plt.ylabel('SSIM')
#     plt.title(f"{title} - SSIM")
#     plt.legend()
#     plt.grid(True)
#     plt.show()
# # Plot psnr comparison
#     plt.figure(figsize=(12, 8))
#     plt.plot(data1['psnr'], label=f'{label1}_psnr', alpha=0.7, color='blue')
#     plt.plot(data2['psnr'], label=f'{label2}_psnr', alpha=0.7, color='orange')
#     plt.xlabel('Frame')
#     plt.ylabel('PSNR')
#     plt.title(f"{title} - PSNR")
#     plt.legend()
#     plt.grid(True)
#     plt.show()

#     # Plot Duration comparison
#     plt.figure(figsize=(12, 8))
#     plt.plot(data1['duration'], label=f'{label1}_duration', alpha=0.7, color='red')
#     plt.plot(data2['duration'], label=f'{label2}_duration', alpha=0.7, color='blue')
#     plt.xlabel('Frame')
#     plt.ylabel('Duration')
#     plt.title(f"{title} - Duration")
#     plt.legend()
#     plt.grid(True)
#     plt.show()

# Function to calculate the average of a list
def calculate_average(values):
    return sum(values) / len(values) if values else 0

# Get all param_n_ prefixes from the current directory
files = [f for f in os.listdir('.') if f.startswith('param_') and f.endswith('.json')]
prefixes = set('_'.join(f.split('_')[:2]) + '_' for f in files)
print("Detected files:", files)
print("Detected prefixes:", prefixes)

if len(prefixes) < 2:
    print("Not enough parameter sets to compare. Please ensure at least two param_n_ sets are present.")
else:
    print("Available parameter sets:")
    for i, prefix in enumerate(prefixes, 1):
        print(f"{i}. {prefix}")

    try:
        # Load data for all parameter sets
        data_list = []
        labels = []
        for prefix in prefixes:
            try:
                data = {
                    'size': load_json(f'{prefix}size.json'),
                    'ssim': load_json(f'{prefix}ssim.json'),
                    'psnr': load_json(f'{prefix}psnr.json'),
                    'duration': load_json(f'{prefix}duration.json')
                }
                data_list.append(data)
                labels.append(prefix)
            except FileNotFoundError as e:
                print(f"Warning: Missing file for prefix {prefix}. Skipping this parameter set.")

        # Calculate and print averages for all parameter sets
        for label, data in zip(labels, data_list):
            avg_ssim = calculate_average(data['ssim'])
            avg_psnr = calculate_average(data['psnr'])
            print(f"Average SSIM for {label}: {avg_ssim}")
            print(f"Average PSNR for {label}: {avg_psnr}")

        # Plot all comparisons
        plot_all_comparisons(data_list, labels, "Comparison of All Parameter Sets")
    except ValueError:
        print("Invalid input. Please enter a number.")   

    # try:
    #     # Let the user select two parameter sets for comparison
    #     choice1 = int(input("Select the first parameter set (enter the number): ")) - 1
    #     choice2 = int(input("Select the second parameter set (enter the number): ")) - 1

    #     if choice1 < 0 or choice2 < 0 or choice1 >= len(prefixes) or choice2 >= len(prefixes):
    #         print("Invalid selection. Please try again.")
    #     else:
    #         # Get the selected prefixes
    #         prefix_list = list(prefixes)
    #         prefix1 = prefix_list[choice1]
    #         prefix2 = prefix_list[choice2]

    #         # Load data for the selected prefixes
    #         data1 = {
    #             'size': load_json(f'{prefix1}size.json'),
    #             'ssim': load_json(f'{prefix1}ssim.json'),
    #             'psnr': load_json(f'{prefix1}psnr.json'),
    #             'duration': load_json(f'{prefix1}duration.json')
    #         }
    #         data2 = {
    #             'size': load_json(f'{prefix2}size.json'),
    #             'ssim': load_json(f'{prefix2}ssim.json'),
    #             'psnr': load_json(f'{prefix2}psnr.json'),
    #             'duration': load_json(f'{prefix2}duration.json')
    #         }

    #         # Calculate and print averages
    #         avg_ssim1 = calculate_average(data1['ssim'])
    #         avg_ssim2 = calculate_average(data2['ssim'])
    #         avg_psnr1 = calculate_average(data1['psnr'])
    #         avg_psnr2 = calculate_average(data2['psnr'])
    #         print(f"Average SSIM for {prefix1}: {avg_ssim1}")
    #         print(f"Average SSIM for {prefix2}: {avg_ssim2}")
    #         print(f"Average PSNR for {prefix1}: {avg_psnr1}")
    #         print(f"Average PSNR for {prefix2}: {avg_psnr2}")

    #         # Calculate and print the difference
    #         ssim_difference = (avg_ssim1 - avg_ssim2) / 100
    #         psnr_difference = avg_psnr1 - avg_psnr2
    #         print(f"Difference in average SSIM: {ssim_difference}%")
    #         print(f"Difference in average PSNR: {psnr_difference} dB")

    #         # Calculate and print the maximum difference
    #         max_ssim_difference = max(abs(a - b) for a, b in zip(data1['ssim'], data2['ssim']))
    #         max_psnr_difference = max(abs(a - b) for a, b in zip(data1['psnr'], data2['psnr']))
    #         print(f"Maximum difference in SSIM values: {max_ssim_difference}")
    #         print(f"Maximum difference in PSNR values: {max_psnr_difference}")

    #         # Plot the comparison
    #         plot_comparison(data1, data2, prefix1, prefix2, f"Comparison of {prefix1} and {prefix2}")
    # except ValueError:
    #     print("Invalid input. Please enter a number.")