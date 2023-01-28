import numpy as np


def sw_dtw_pe(x, y, n ,w, nw):
    min3 = min(n, w, nw)
    cost = abs(np.int16(x) - np.int16(y))
    score = cost + min3
    return score

def sw_dtw_datapath(squiggle_list, reference_list):
    ref_len = len(reference_list)

    # Initialize the cost matrix
    cost_matrix = np.zeros((len(squiggle_list), len(reference_list)), dtype=np.uint16)
    # print(cost_matrix)
    
    # Initialize the first row and column of the cost matrix
    for i in range(0, len(reference_list)):
        cost_matrix[0, i] = sw_dtw_pe(squiggle_list[0], reference_list[i], 0, 0, 0);
    for i in range(1, len(squiggle_list)):
        cost_matrix[i, 0] = sw_dtw_pe(squiggle_list[i], reference_list[0], cost_matrix[i - 1, 0], np.inf, np.inf);
        if (i < 3):
            print(cost_matrix[i, 0])
    
    # Fill in the cost matrix
    for i in range(1, len(squiggle_list)):
        for j in range(1, ref_len):
            cost_matrix[i, j] = sw_dtw_pe(squiggle_list[i], reference_list[j], cost_matrix[i - 1, j], cost_matrix[i, j - 1], cost_matrix[i - 1, j - 1])

    # Parse through the last row
    min_cost = cost_matrix[len(squiggle_list) - 1, 0]
    print(cost_matrix)
    min_pos = 0
    for j in range(1, ref_len):
        if cost_matrix[len(squiggle_list) - 1, j] < min_cost:
            min_cost = cost_matrix[len(squiggle_list) - 1, j]
            min_pos = j


    # output cost matrix to ascii file for debugging; x-axis is reference (left to right is small to big), y-axis is squiggle (top to bottom is small to big)
    with open("cost_matrix.dat", "w") as f:
        for i in range(0, len(squiggle_list)):
            for j in range(0, ref_len):
                f.write(str(cost_matrix[i, j]) + "\t")
            f.write("\n")

    return min_cost, min_pos

def gen_squiggle_list(size):
    squiggle_list = np.random.randint(0, 100, size=size, dtype=np.int16)
    squiggle_list.tofile("squiggle_list_bin.dat")
    # output to ascii file for debugging
    with open("squiggle_list.dat", "w") as f:
        for i in range(0, size):
            f.write(str(squiggle_list[i]) + "\n")

def gen_reference_list(size):
    reference_list = np.random.randint(0, 100, size=size, dtype=np.int16)
    reference_list.tofile("reference_list_bin.dat")
    # output to ascii file for debugging
    with open("reference_list.dat", "w") as f:
        for i in range(0, size):
            f.write(str(reference_list[i]) + "\n")

def get_dbg_squiggle_list(filename):
    with open(filename, "r") as f:
        squiggle_list = []
        for line in f:
            squiggle_list.append(int(line))

    return squiggle_list

def get_dbg_reference_list(filename):
    with open(filename, "r") as f:
        reference_list = []
        for line in f:
            reference_list.append(int(line))

    return reference_list


# Main
def main():
    # Generate data
    # gen_squiggle_list(5)
    # gen_reference_list(25)

    # Read in the squiggle and reference lists
    squiggle_list = get_dbg_squiggle_list("squiggle_list.dat")
    reference_list = get_dbg_reference_list("reference_list.dat")

    ref_len = len(reference_list)
    print("Squiggle_size: " + str(len(squiggle_list)))
    print("Squirgle_list: " + str(squiggle_list))
    print("Reference_size: " + str(ref_len))
    print("Reference_list: " + str(reference_list))
    
    # Run the datapath
    min_cost, min_pos = sw_dtw_datapath(squiggle_list, reference_list)
    
    # Print the results
    print("Minimum cost: " + str(min_cost))
    print("Minimum position: " + str(min_pos))


if __name__ == "__main__":
    main()