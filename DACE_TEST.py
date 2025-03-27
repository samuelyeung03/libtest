import matplotlib.pyplot as plt

# File paths
complexity_file = "./complexity.txt"
duration_file = "./duration.txt"

# Read data from files
def read_data(file_path):
    with open(file_path, "r") as file:
        return [int(line.strip()) for line in file if line.strip().isdigit()]

complexity = read_data(complexity_file)
duration = read_data(duration_file)

y = 1000000 / 45

# Plotting the graph
plt.figure(figsize=(10, 6))
plt.plot(complexity, label="Complexity", color="blue")
plt.plot(duration, label="Duration", color="orange")
plt.axhline(y, label="Y = 100000/45", color="green", linestyle="--")
plt.xlabel("Index")
plt.ylabel("Value")
plt.title("Complexity vs Duration")
plt.legend()
plt.grid(True)
plt.show()