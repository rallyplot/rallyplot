from rallyplot import Plotter
import numpy as np

def test_instantiate_plotter():
    """
    Quick test to check importing and instantiation of `Plotter` leads
    to not errors (e.g. shared library location errors, etc).
    """
    plotter = Plotter(anti_aliasing_samples=4, color_mode="light")
    print("Successfully imported and initialised Plotter.")

test_instantiate_plotter()