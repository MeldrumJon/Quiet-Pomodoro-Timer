# Creates a gamma-corrected lookup table
# Modified from https://gist.github.com/hexagon5un/3df734ad08d8dc8d9ace0491ef97cc58

import math

# Returns a (exponential? logarthimic?) curve from 0 to 1
# @param nsteps  Number of steps in the curve.
# @param gamma   Gamma to use
def gamma(nsteps, gamma):
	gammaedUp = [math.pow(x, gamma) for x in range(nsteps)]
	return [x/max(gammaedUp) for x in gammaedUp]

# Scales an array of values from 0 to an array of values 0 to max.  
# Rounds the numbers to the nearest base.
# @param max     Max value for the array
# @param base    Round to the nearest base.  Use 1 for normal integer rounding.
# @param gammas  The array of values between 0 and 1.
def rounder(max, base, gammas):
	return [int(base*round((x*max)/base)) for x in gammas]

if __name__ == "__main__":
	output = file("gamma.h", "w")
	gammaValue = 2.3 # usually around 2, look up online
	brightnessSteps = 20
	gammaUnscaled = gamma(brightnessSteps, gammaValue)
	gammaScaled = rounder(255, 1, gammaUnscaled) # between 0 and 255, rounded to nearest 1

	output.write("/* Brightness table: gamma = %s */ \n\n" % gammaValue)
	for steps in [9, 10, 11, 12, 13, 14, 15]:
		array = []
		shadesSteps = steps # total shades, +1 for zero
		shadesUnscaled = gamma(shadesSteps, gammaValue)
		for i in range(brightnessSteps):
			if gammaScaled[i] < 4:
				continue
			shades = rounder(gammaScaled[i], 1, shadesUnscaled)
			array.append(shades)
		
		output.write("const uint8_t gamma_table[%d][%d] = {\n" % (len(array), shadesSteps))
		for list in array:
			output.write("\t{")
			for value in list:
				output.write(" %d," % value)
			output.write("}\n")
		output.write("};\n")
	output.close()