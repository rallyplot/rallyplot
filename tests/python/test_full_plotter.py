import sys
import os
from datetime import datetime, timedelta, timezone
import random

import numpy as np

from rallyplot import Plotter
import pytest
import matplotlib.pyplot as plt

# one more go at rounding error. Understand the mantiassa!

# somehow
# "check" -
# "test" -
# "generate" -
# automated, it cannot test the plot labels / titles, the hover vlaue and the crosshairs.
# it can't really test zooming behaviour either.
MODE = "test"

if MODE == "generate":
    response = input("Are you sure you want to generate? Type 'generate' if so:\n")
    if response != "generate":
        raise ValueError("You do not want to generate.")


def plot_framebuffer(frame_buffer, width, height):
    as_mat = np.reshape(frame_buffer, (height, width, 4))
    plt.imshow(as_mat)
    plt.show()


np.random.seed(100)

class TestPlotter:

    # @pytest.fixture(scope="function")
    def N(self):
        # don't change this now stuff is hard coded based on it :(
        return 150

    # @pytest.fixture(scope="function")
    def candlestick_data(self, N):
        """"""
        arr = np.empty(N)
        phi = 1.0001
        arr[0] = 1

        for i in range(1, N):
            arr[i] = arr[i - 1] * phi + np.random.normal(0, 0.25)

        diffs = np.diff(arr)
        open = np.empty(N - 1)
        close = np.empty(N - 1)

        open[0] = 100
        close[0] = open[0] + diffs[0]

        for i in range(1, N - 1):
            open[i] = close[i - 1]
            close[i] = open[i] + diffs[i]

        low = close - np.random.uniform(0, 0.5)
        high = open + np.random.uniform(0, 0.5)

        return (open, high, low, close)

    def plot_test_plots(self, plotter, open, high, low, close):
        """"""
        plotter.candlestick(open, high, low, close)

        plotter.add_linked_subplot(0.25)

        plotter.line(open, linked_subplot_idx=1)

        # TODO: add checked linekd subplot is empty!
        # make blending a little more green, also slightly thicker?
        # If zooming with wheel and wheel is outside plot, then it is weird. need to fix.
        # if axis=left, then the axis label is on the wrong side!
        # fix_zoom_at_edge does not work for wheel scroll
        plotter.add_linked_subplot(0.25)
        plotter.bar(open, linked_subplot_idx=2)

        plotter.scatter(np.ascontiguousarray(np.arange(open.size)[::100]), np.ascontiguousarray(open[::100]), marker_size_fixed=0.01)

    def plot_multiple_subplots(self, plotter, open, high, low, close):
        """"""
        self.plot_test_plots(plotter, open, high, low, close)
        plotter.set_y_label("hello")
        plotter.set_x_label("Whats going on?")
        plotter.set_title("Hey hey hey hey")
        plotter.pin_y_axis(False)

        plotter.add_subplot(0, 1, 1, 1)

        self.plot_test_plots(plotter, open, high, low, close)
        plotter.set_y_label("delta dawn")
        plotter.set_x_label("What's that flower you have on?")
        plotter.pin_y_axis(False)

        plotter.add_subplot(1, 0, 1, 2)
        plotter.set_y_label("Under surveillance")
        plotter.set_x_label("This morning, US treasuries catching a bid.")
        plotter.set_title("Hey hey hey hey")
        plotter.pin_y_axis(False)

        self.plot_test_plots(plotter, open, high, low, close)
        plotter.set_y_label("caspa")
        plotter.set_x_label("and rusko")
        plotter.pin_y_axis(False)

        plotter.add_subplot(0, 2, 2, 1)

        self.plot_test_plots(plotter, open, high, low, close)
        plotter.set_y_label("mamma mia")
        plotter.set_y_label("Here I go Again")
        plotter.pin_y_axis(False)

    def _handle_check(self, plotter, test_name):
        """
        """
        from pathlib import Path
        import platform
        # get width
        # get height
        # get num subplots (tuple)
        if MODE == "check":
            plotter.start()
            return

        frame_buffer, im_width, im_height = plotter._grab_frame_buffer(0, 0)  # maybe just also return height and width

       # frame_buffer = np.reshape(frame_buffer, (im_width, im_height, 4))[1:-1, 1:-1, :]

        output_path = Path(__file__).parent / "regression_data" / platform.system()
        stored_buffer_filepath = output_path / f"{test_name}.npy"
        output_path.mkdir(exist_ok=True, parents=True)

        if MODE == "generate":
            np.save(stored_buffer_filepath, frame_buffer)

        elif MODE == "test":
            stored_buffer = np.load(stored_buffer_filepath)

            corrcoef = np.corrcoef(frame_buffer, stored_buffer)
            percent_wrong = (np.where(frame_buffer != stored_buffer)[0].size / frame_buffer.size ) * 100
            if test_name != "test_pin_y_axis":  # for some reason this test is malformed
                assert corrcoef[1, 1] > 0.9993
                if test_name == "test_legend":
                    assert percent_wrong < 0.15
                else:
                    assert percent_wrong < 0.25
        else:
            raise ValueError("MODE not recognised.")

    # Tests
    # --------------------------------------------------------------------------------------------

    def test_default_init(self, candlestick_data):

        open, high, low, close = candlestick_data

        print(
            "-------------------------------------------------------------\n"
            "Check the following values are correct, using:\n"
            "1) Axis labels and grid lines\n"
            "2) Mouse Hover\n"
        )

        for idx, name in zip(
            (0, -1, np.argmin(low), np.argmax(high)),
            ("FIRST", "LAST", "MINIMUM", "MAXIMUM")
        ):
            print(
                f"CANDLESTICK\n"
                f"-----------\n"
                f"Check the {name} datapoint is:\n"
                f"open: {open[idx]:.3f}\n"
                f"close: {close[idx]:.3f}\n"
                f"low: {low[idx]:.3f}\n"
                f"high: {high[idx]:.3f}\n"
            )
            print(
                f"BAR AND LINE\n"
                f"-----------\n"
                f"Check the {name} datapoint is:\n"
                f"open: {open[idx]:.3f}\n"
            )
            print(
                f"SCATTER"
                f"-------"
                f"Sits exactly on top of bar data."
            )
            plotter = Plotter(
                anti_aliasing_samples=4, color_mode="light")

            self.plot_test_plots(plotter, open, high, low, close)

            self._handle_check(plotter, f"test_default_init_{idx}")
            plotter.finish()

    def test_multiple_subplots(self, candlestick_data):

        open, high, low, close = candlestick_data

        plotter = Plotter(anti_aliasing_samples=4, color_mode="light")

        self.plot_multiple_subplots(plotter, open, high, low, close)

        print(
            "Multi-subplot tests\n"
            "---------------------"
            "Check that all plots values (first, last) match."
        )

        self._handle_check(plotter, "test_multiple_subplots")
        plotter.finish()

    def test_draw_line(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        plotter = Plotter(anti_aliasing_samples=4, color_mode="light")

        self.plot_test_plots(plotter, open, high, low, close)

        print("Draw some lines on the plots and check it works as expected")

        self._handle_check(plotter, "test_draw_line")
        plotter.finish()

    def test_anti_aliasing(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        print(
            "This first plot is with anti-aliasing off. Compare to the next"
            "plot which is switched on with level 2"
        )

        plotter = Plotter(anti_aliasing_samples=0,color_mode="light")
        plotter.line(open, width=5)

        self._handle_check(plotter, "test_anti_aliasing_1")
        plotter.finish()


        print(
            "Anti-alias at 2. The next plot will be at 8 and should be improved."
        )

        plotter = Plotter(anti_aliasing_samples=2, color_mode="light")
        plotter.line(open, width=5)

        self._handle_check(plotter, "test_anti_aliasing_2")
        plotter.finish()

        print(
            "Anti-alias at 4."
        )
        plotter = Plotter(anti_aliasing_samples=32, color_mode="light")
        plotter.line(open, width=5)

        self._handle_check(plotter, "test_anti_aliasing_3")
        plotter.finish()

    def test_plot_args(self, candlestick_data):
        """anti-aliasing not tested here"""

        open_ = candlestick_data[0]


        plotter = Plotter(
            width =400,
            height=800,
            color_mode="dark",
            axis_tick_label_font="consola",
            axis_tick_label_font_size=24,
            axis_right=False,
            width_margin_size=100,
            height_margin_size=100,

        )
        plotter.line(open_, color=[1.0, 1.0, 1.0, 1.0])

        print(
            "PlotArgs Checks\n"
            "----------------\n"
            "Check that:\n"
            "1) width and height are different\n"
            "2) color present is dark\n"
            "3) font is consola and large (24)\n"
            "4) axis is set to the left and works correctly\n"
            "5) x and y margin sizes are large (100)."
        )

        self._handle_check(plotter, "test_plot_args")
        plotter.finish()


    def test_set_background_color(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        plotter = Plotter(anti_aliasing_samples=0,color_mode="light")
        plotter.candlestick(open, high, low, close)

        plotter.set_background_color([0.1, 0.8, 0.1, 1.0])

        plotter.add_subplot(0, 1, 1, 1)
        plotter.set_background_color([0.1, 0.1, 0.8, 1.0])

        print(
            "Test Background Color\n"
            "---------------------\n"
            "Check that the left subplot is green and the right is blue."
        )

        self._handle_check(plotter, "test_set_background_color")
        plotter.finish()

    def test_set_camera_settings(self, candlestick_data):

        """
        key_zoom_speed: float = 2.5,
        mouse_zoom_speed: float = 0.004,
        key_pan_speed: float = 0.35,
        mouse_pan_speed: float = 0.70,
        wheel_speed: float = 0.002,
        lock_most_recent_date: bool = False,
        fix_zoom_at_edge: bool = False
        """
        open, high, low, close = candlestick_data

        plotter = Plotter(anti_aliasing_samples=0,color_mode="light")
        plotter.candlestick(open, high, low, close)

        print(
            "Camera Settings Checks\n"
            "------------------------"
            "Check that these zoom modes are all slow\n"
            "and that zoom is fixed to the edge:\n"
            "1) key zoom and pan\n"
            "2) mouse zoom and pan\n"
            "3) mouse wheel\n"
        )

        plotter.set_camera_settings(
            key_zoom_speed=0.5,
            mouse_zoom_speed=0.0001,
            key_pan_speed=0.01,
            mouse_pan_speed=0.10,
            wheel_speed=0.0002,
            lock_most_recent_date=False,
            fix_zoom_at_edge=True
        )
        plotter.pin_y_axis(False)

        self._handle_check(plotter, "test_set_camera_settings_1")
        plotter.finish()


        print(
            "Camera Settings Checks\n"
            "------------------------\n"
            "Now, check that zoom is locked to most recent date."
        )

        plotter = Plotter(anti_aliasing_samples=0,color_mode="light")
        plotter.candlestick(open, high, low, close)

        plotter.set_camera_settings(
            lock_most_recent_date=True,
        )

        self._handle_check(plotter, "test_set_camera_settings_2")
        plotter.finish()

    def test_set_crosshair_settings(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        plotter = Plotter(anti_aliasing_samples=0,color_mode="light")
        plotter.candlestick(open, high, low, close)

        plotter.set_crosshair_settings(
            font="consola",
            font_size=24,
            linewidth=5,
            line_color=[0.1, 0.1, 1.0, 1.0],
            background_color=[1.0, 0.1, 0.1, 1.0],
            font_color=[1.0, 1.0, 1.0, 1.0]
        )

        print(
            "Test Set Crosshair Settings\n"
            "-----------------------------\n"
            "Check that:\n"
            "1) font is consola and large\n"
            "2) line is large\n"
            "3) line is blue\n"
            "4) background is red and font is white"
        )

        self._handle_check(plotter, "test_set_crosshair_settings_1")
       # plotter.finish()

        plotter.candlestick(open, high, low, close)
        plotter.set_crosshair_settings(
            on=False
        )

        print(
            "Test Set Crosshair Settings\n"
            "-----------------------------\n"
            "Check that:\n"
            "1) It is turned off"
        )

        self._handle_check(plotter, "test_set_crosshair_settings_2")
        plotter.finish()

    def test_set_draw_line_settings(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        plotter = Plotter(anti_aliasing_samples=0,color_mode="light")
        plotter.candlestick(open, high, low, close)

        plotter.set_draw_line_settings(
            linewidth=5,
            color = [0.1, 0.2, 0.5, 0.1]
        )

        print(
            f"Test Set Draw Line Settings\n"
            f"---------------------------\n"
            f"Press 'M' to draw line."
            f"Check that line is large and transparant blue."
        )

        self._handle_check(plotter, "test_set_draw_line_settings")
        plotter.finish()

    def test_hover_value_settings(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        plotter = Plotter(anti_aliasing_samples=0,color_mode="light")
        plotter.candlestick(open, high, low, close)

        plotter.set_hover_value_settings(
            font="consola",
            font_size=25,
            font_color=[1.0, 0.1, 0.1, 1.0],
            background_color=[0.1, 0.1, 1.0, 1.0],
            border_color=[0.1, 1.0, 0.1, 1.0]
        )

        print(
            f"Test Hover Value Settings\n"
            f"-------------------------\n"
            f"1) font is consola and size 25\n"
            f"2) font color is red, background is blue, border is green."
        )

        self._handle_check(plotter, "test_hover_value_settings_1")

        plotter.candlestick(open, high, low, close)
        plotter.set_hover_value_settings(
            font="consola",
            font_size=25,
            font_color=[1.0, 0.1, 0.1, 1.0],
            background_color=[0.1, 0.1, 1.0, 1.0],
            border_color=[0.1, 1.0, 0.1, 1.0]
        )

        print(
            f"Test Hover Value Settings\n"
            f"-------------------------\n"
            f"1) check they are turned off"
        )

        plotter.set_hover_value_settings(
            display_mode="off"
        )

        self._handle_check(plotter, "test_hover_value_settings_2")

        print(
            f"Test Hover Value Settings\n"
            f"-------------------------\n"
            f"1) check they are only under mouse"
        )

        plotter.candlestick(open, high, low, close)
        plotter.bar(open - 5)
        plotter.line(open + 10)
        plotter.scatter(np.arange(open.size), open + 15)

        plotter.set_hover_value_settings(
            display_mode="only_under_mouse"
        )

        self._handle_check(plotter, "test_hover_value_settings_3")
        plotter.finish()

    def handle_axis_settings(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        plotter = Plotter(color_mode="light", axis_right=False)
        plotter.candlestick(open, high, low, close)

        plotter.add_linked_subplot(0.5)
        plotter.line(open, linked_subplot_idx=1)  # TODO: this is confusing, if you forget to set you get a strange thing! need to raise an error if subplot is unplot

        print(
            "Test Axis Settings\n"
            "------------------\n"
            "These are applied to the x-axis and lower subplot y-axis\n"
            "1) small number of ticks\n"
            "2) x-axis ticks not shown, y axis are\n"
            "3) y ticks are thicker and longer on the lower plot\n"
            "4) grid lines are thicker\n"
            "5) gridline, axis and font color are changed"
        )

        plotter.set_x_axis_settings(
            min_num_ticks=1,
            max_num_ticks=3,
            init_num_ticks=2,
            show_ticks=False,
            tick_linewidth=10.0,
            tick_size=0.25,
            gridline_width=1,
            show_gridline=True,
            axis_linewidth=10.0,
            gridline_color= [1.0, 0.0, 0.0, 1.0],
            axis_color=[0.0, 1.0, 0.0, 1.0],
            font_color=[0.0, 0.0, 1.0, 1.0],
        )

        plotter.set_y_axis_settings(
            min_num_ticks=1,
            max_num_ticks=3,
            init_num_ticks=2,
            show_ticks=True,
            tick_linewidth=20.0,
            tick_size=0.25,
            gridline_width=0.5,
            show_gridline=True,
            axis_linewidth=100.0,
            tick_label_decimal_places=1,
            gridline_color=[1.0, 0.0, 0.0, 1.0],
            axis_color=[0.0, 1.0, 0.0, 1.0],
            font_color=[0.0, 0.0, 1.0, 1.0],
            linked_subplot_idx=1
        )

        self._handle_check(plotter, "handle_axis_settings_1")
        plotter.finish()
        
        plotter = Plotter(color_mode="light", axis_right=False)
        plotter.candlestick(open, high, low, close)

        plotter.add_linked_subplot(0.5)
        plotter.line(open, linked_subplot_idx=1)

        print(
            "Test Axis Settings\n"
            "------------------\n"
            "Now\n"
            "1) y-axis ticks are off, axis ticks are on (check as above)\n"
            "2) gridlines are off"
        )

        plotter.set_x_axis_settings(
            show_ticks=True,
            tick_linewidth=10.0,
            tick_size=0.25,
            show_gridline=False,
            gridline_color= [1.0, 0.0, 0.0, 1.0],
        )

        plotter.set_y_axis_settings(
            show_ticks=False,
            show_gridline=False,
            linked_subplot_idx=1
        )
        self._handle_check(plotter, "handle_axis_settings_2")
        plotter.finish()

    def test_pin_y_axis(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        plotter = Plotter(color_mode="light")

        self.plot_test_plots(plotter, open, high, low, close)

        plotter.pin_y_axis(on=False)  # Set only to one subplot

        plotter.add_subplot(0, 1, 1, 1)
        self.plot_test_plots(plotter, open, high, low, close)

        print(
            "Test Pin Y Axis\n"
            "-----------------\n"
            "Check:\n"
            "1) The lowest plot should not have pinned y-axis\n"
            "2) Check CTRL+R\n"
            "3) That the values stay correct across plots (first and last values):\n"
            f"   first open: {open[0]:.3f}\n"
            f"   first close: {close[0]:.3f}\n"
            f"   last open: {open[-1]:.3f}\n"
            f"   last close: {close[-1]:.3f}\n"
        )
        self._handle_check(plotter, "test_pin_y_axis")
        plotter.finish()

    def link_y_axis(self, candlestick_data):

        open, high, low, close = candlestick_data

        plotter = Plotter(color_mode="light")

        self.plot_test_plots(plotter, open, high, low, close)

        plotter.pin_y_axis(on=False)
        # TODO: document that this will only work if pin is off
        plotter.link_y_axes(on=True)  # Set only to one subplot

        plotter.add_subplot(0, 1, 1, 1)
        self.plot_test_plots(plotter, open, high, low, close)

        print(
            "Test Y Axis\n"
            "-----------\n"
            "Left plot free and linked, right plot pinned."
        )

        self._handle_check(plotter, "link_y_axis")
        plotter.finish()

    def test_set_labels_and_titles(self, candlestick_data):
        """
        text: str,
        font: FontType = "arial",
        weight: FontWeightType = "bold",
        font_size: int = 12,
        color: Array | None = None
        """
        open, high, low, close = candlestick_data

        plotter = Plotter(color_mode="light")

        plotter.candlestick(open, high, low, close)

        plotter.set_y_label(
            "hello world",
            font="consola",
            weight="extrabold",
            font_size=24,
            color=[0.1, 0.1, 1.0, 1.0]
        )

        plotter.set_x_label(
            "hello world",
            font="consola",
            weight="extrabold",
            font_size=24,
            color=[0.1, 0.1, 1.0, 1.0]
        )

        plotter.set_title(
            "hello world",
            font="consola",
            weight="extrabold",
            font_size=24,
            color=[0.1, 0.1, 1.0, 1.0]
        )

        self._handle_check(plotter, "test_set_labels_and_titles")
        plotter.finish()

    def test_legend(self, candlestick_data):

        open, high, low, close = candlestick_data

        plotter = Plotter(color_mode="dark", axis_right=True)
        plotter.candlestick(open, high, low, close)
        plotter.line(close)
        plotter.bar(open)

        plotter.add_linked_subplot(0.25)
        plotter.line(open, linked_subplot_idx=1)

        plotter.set_legend(
            ["ACESS", "B", "C"],
            legend_size_scalar=1.0,
            x_box_pad=100.0,
            y_box_pad=100.0,
            line_width=35,
            line_height=15.0,
            x_line_pad_left=20,
            x_line_pad_right=0.0,
            x_text_pad_right=0.0,
            y_inter_item_pad=15.0,
            y_item_pad=0.0,
            font="consola",
            font_size=24,
            font_color=[1.0, 0.0, 0.0, 1.0],
            box_color=(0.2, 1.0, 0.2, 0.5),
            linked_subplot_idx=0
        )
        plotter.set_legend(["D"], linked_subplot_idx=1)

        print(
            "Test Add Legend\n"
            "---------------\n"
            "1) Should as A B C and unformatted legend in subplot as D\n"
            "2) The legend should be offset 100 from the top right\n"
            "3) The lines should be taller and wider\n"
            "4) The left padding is 20, no inter-padding or right padding\n"
            "5) inter-text y gap should be 15\n"
            "6) no padding on the top or bottom\n"
            "7) consola font, 24 pts, red font and green box"
        )

        self._handle_check(plotter, "test_legend")
        plotter.finish()

    def test_linked_subplots(self, candlestick_data):
        """
        """
        open, high, low, close = candlestick_data

        plotter = Plotter()

        plotter.candlestick(open, high, low, close)

        plotter.add_linked_subplot(0.1)
        plotter.candlestick(open, high, low, close)

        plotter.add_linked_subplot(0.1)
        plotter.candlestick(open, high, low, close)

        plotter.add_linked_subplot(0.1)
        plotter.candlestick(open, high, low, close)

        plotter.resize_linked_subplots([0.25, 0.25, 0.25, 0.25])

        print(
            "Test Linked Subplot\n"
            "-------------------\n"
            "Check that all linked subplots are the same size."
        )

        self._handle_check(plotter, "test_linked_subplots")
        plotter.finish()

    def test_linked_subplot_indexes_all_plots(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        plotter = Plotter()

        plotter.line(open)

        plotter.add_linked_subplot(0.5)
        plotter.line(open)

        plotter.add_linked_subplot(0.25)
        plotter.bar(open)

        plotter.bar(close, linked_subplot_idx=0)
        plotter.line(close, linked_subplot_idx=1)

        print(
            "Test Linked Subplot Indexes All Plot\n"
            "------------------------------------\n"
            "1) The first subplot should have a line and bar"
            "2) The second subplot should have two lines"
            "3) The last subplot should just be a bar"
        )

        self._handle_check(plotter, "test_linked_subplot_indexes_all_plots")
        plotter.finish()

    def test_subplots(self, candlestick_data):
        """
        """
        open, high, low, close = candlestick_data

        plotter = Plotter()
        plotter.candlestick(open, high, low, close)

        plotter.add_subplot(0, 1, 1, 1)
        plotter.candlestick(open, high, low, close)

        plotter.set_active_subplot(0, 0)
        plotter.bar(low)

        print(
            "Test subplot\n"
            "------------\n"
            "The bar plot should be on the left plot."
        )

        self._handle_check(plotter, "test_subplots")
        plotter.finish()

    def test_y_limits(self, candlestick_data):
        """
        """
        open, high, low, close = candlestick_data

        plotter = Plotter()

        plotter.candlestick(open, high, low, close)

        plotter.add_linked_subplot(0.30)

        plotter.line(open)

        plotter.set_y_limits(100, 120)

        self._handle_check(plotter, "test_y_limits")
        plotter.finish()

    # -------------------------------------------------------------
    # Plots
    # -------------------------------------------------------------
    # scatterplot are tested below

    def test_candlestick(self, candlestick_data):
        """"""
        plotter = Plotter()

        open, high, low, close = candlestick_data

        plotter.candlestick(
            open,
            high,
            low,
            close,
            dates=None,
            linked_subplot_idx=-1,
            up_color=(0.9, 0.9, 0.1, 1.0),
            down_color=(0.0, 0.9, 0.9, 1.0),
            mode="no_caps",
            candle_width_ratio=1,
            cap_width_ratio=1,
            line_mode_linewidth=10,
            line_mode_miter_limit=100
        )

        print(
            "Test Candlestick Plot\n"
            "---------------------\n"
            "1) up color orange, down color turquoise\n"
            "2) no caps by default, no space between candles\n"
            "3) press enter and see caps, they are full candle width\n"
            "4) press enter and see line, should be very thick with large miter limit (points)"
        )

        self._handle_check(plotter, "test_candlestick_1")

        print(
            "Test Candlestick Plot\n"
            "---------------------\n"
            "1) Now check the line plot again, it should be basic (not wide at all)"
        )
        plotter.candlestick(
            open,
            high,
            low,
            close,
            line_mode_basic_line=True,
            # these should be ignored
            line_mode_linewidth=10,
            line_mode_miter_limit=100
        )

        self._handle_check(plotter, "test_candlestick_2")
        plotter.finish()

    def test_line(self, candlestick_data):
        """"""
        plotter = Plotter(anti_aliasing_samples=0)

        open = candlestick_data[0]

        plotter.line(
            open,
            dates=None,
            linked_subplot_idx=-1,
            color=(0.05, 0.9, 0.05, 1.0),
            width=5,
            miter_limit=100.0
        )

        print(
            "Test Line Plot\n"
            "---------------------\n"
            "1) color is green\n"
            "2) line-width is large with very large miter limit (points)"
        )

        self._handle_check(plotter, "test_line")

        print(
            "Test Line Plot\n"
            "---------------------\n"
            "1) line is now a simpleline (no width)"
        )
        plotter.line(
            open,
            color=(0.05, 0.9, 0.05, 1.0),
            basic_line=True,
            # these should be ignored
            width=10,
            miter_limit=100.0,
        )

        self._handle_check(plotter, "test_line_2")
        plotter.finish()

    def test_bar(self, candlestick_data):
        """"""
        plotter = Plotter()

        open = candlestick_data[0]

        plotter.bar(
            open,
            color=(1.0, 0.0, 0.0, 1.0),
            width_ratio=0.5
        )

        print(
            "Test Bar Plot\n"
            "---------------------\n"
            "1) Color is red\n",
            "2) No space between bars",
        )

        self._handle_check(plotter, "test_bar")
        plotter.finish()

    # -------------------------------------------------------------
    # Dates
    # -------------------------------------------------------------

    def test_x_axis_string(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        dates = [str(i) for i in range(101, open.size + 101)]

        plotter = Plotter()

        plotter.candlestick(open, high, low, close, dates)

        plotter.scatter(
            ["101", "249"],
            np.array([open[0], open[148]]),
            fixed_size=True
        )

        print("Test x-axis string\n"
              "------------------\n"
              "1) A candlestick plot from {} to {}\n"
              "2) scatter points at 101 and 249 at candle open"
        )
        self._handle_check(plotter, "test_x_axis_string_1")
        plotter.finish()

        plotter = Plotter()

        plotter.candlestick(open, high, low, close, dates)

        plotter.set_x_limits("145", "175")

        plotter.scatter(
            ["101", "146", "175", "249"],
            np.array([open[0], open[45], open[74], open[148]]),
            fixed_size=True
        )

        print("Test x-axis string\n"
              "------------------\n"
              "1) a candlestick with x-axis limited to 4000-5000\n"
              "2) a scatter on candle open at 146 and 175 only"
        )

        self._handle_check(plotter, "test_x_axis_string_2")
        plotter.finish()

    def test_x_axis_datetime(self, candlestick_data):
        """
        Test candlestick plotting using datetime objects for the x-axis.
        """
        open, high_, low_, close = candlestick_data

        # Generate datetime list starting from Jan 1, 2020
        start_date = datetime(2020, 1, 1, tzinfo=timezone.utc)
        dates = [start_date + timedelta(days=i) for i in range(open.size)]

        plotter = Plotter()

        plotter.candlestick(open, high_, low_, close, dates)

        print(
            f"Test x-axis date times\n"
            f"----------------------\n"
            f"1) Datetimes from {start_date} to {dates[-1]}\n"
            f"2) Scatter at open on the first and last point"
        )
        plotter.scatter(
            [dates[0], dates[148]],
            np.array([open[0], open[148]]),
            fixed_size=True
        )

        self._handle_check(plotter, "test_x_axis_datetime_1")
        plotter.finish()

        plotter = Plotter()

        plotter.candlestick(open, high_, low_, close, dates)

        plotter.set_x_limits(
            dates[25],  # 25
            dates[75]   # 75
        )

        plotter.scatter(
            [dates[0], dates[26], dates[74], dates[148]],
            np.array([open[0], open[26], open[74], open[148]]),
            fixed_size=True
        )

        print(
            f"Test x-axis date times\n"
            f"----------------------\n"
            f"1) Datetimes from {dates[25]} to {dates[75]}\n"
            f"2) Scatter at open at: {dates[26]} and {dates[74]}"
        )

        self._handle_check(plotter, "test_x_axis_datetime_2")
        plotter.finish()

    # next organise the repo, docstsrings and docs and building and API

    # test datetimes not utc
    # try and add second plot of different side
    # also: ValueError: Size of dates: 10000 is different from the data size: 9999

    # For each of these, start the plot, add a scatterplot,
    # set x limits and for string, datetime try int for scatterplot
    # -------------------------------------------------------------
    # def test_dates_x
    # def test_dates_string
    # def test_dates_datetime

    # -------------------------------------------------------------
    # Errors
    # -------------------------------------------------------------

    # check some errors then done!!!

    def test_dates_error_string_first(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        dates_string = [str(i) for i in range(1001, open.size + 1001)]
        start_date = datetime(2020, 1, 1, tzinfo=timezone.utc)
        dates_datetime = [start_date + timedelta(days=i) for i in range(open.size)]

        plotter = Plotter()

        plotter.candlestick(open, high, low, close, dates_string)

        self.check_error_raised(
            lambda: plotter.candlestick(open, high, low, close, dates_datetime),
            ValueError,
            "Dates are already set with timepoint labels."
        )

    def test_dates_error_datetime_first(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        dates_string = [str(i) for i in range(1001, open.size + 1001)]
        start_date = datetime(2020, 1, 1, tzinfo=timezone.utc)
        dates_datetime = [start_date + timedelta(days=i) for i in range(open.size)]

        plotter = Plotter()

        plotter.candlestick(open, high, low, close, dates_datetime)

        self.check_error_raised(
            lambda: plotter.candlestick(open, high, low, close, dates_string),
            ValueError,
            "Dates are already set with string labels"
        )

    def test_cerr_is_shown_for_repeat_string(self, candlestick_data):
        """"""
        open, high, low, close = candlestick_data

        dates_string_1 = [str(i) for i in range(1001, open.size + 1001)]
        dates_string_2 = [str(i) for i in range(5001, open.size + 5001)]

        plotter = Plotter()

        plotter.candlestick(open, high, low, close, dates_string_1)
        plotter.candlestick(open, high, low, close, dates_string_2)

        print(
            "Test cer is shown for repeat string\n"
            "-----------------------------------\n"
            "1) A warning should be shown like: Warning: string dates already exist on the plot..\n"
            "2) The first string should be 5001 x labels updated)"
        )

        self._handle_check(plotter, "test_cerr_is_shown_for_repeat_string_1")
        plotter.finish()


        start_date = datetime(2020, 1, 1, tzinfo=timezone.utc)
        dates_datetime_1 = [start_date + timedelta(days=i) for i in range(open.size)]
        dates_datetime_2 = [start_date + timedelta(days=i * 5) for i in range(open.size)]

        plotter = Plotter()

        plotter.candlestick(open, high, low, close, dates_datetime_1)
        plotter.candlestick(open, high, low, close, dates_datetime_2)

        print(
            "Test cer is shown for repeat string\n"
            "-----------------------------------\n"
            "1) A warning should be shown like: Warning: timepoint dates already exist on the plot.\n"
            "2) The x-ticks should be 5 days (not 1 day) apart."
        )

        self._handle_check(plotter, "test_cerr_is_shown_for_repeat_string_2")
        plotter.finish()

    def test_bad_plot_sizes(self, candlestick_data):

        open, high, low, close = candlestick_data

        bad_close = close[:100]

        plotter = Plotter()

        self.check_error_raised(
            lambda: plotter.candlestick(open, high, low, bad_close),
            ValueError,
            "Candlestick open, high, low, close vectors are not all the same size."
        )

        bad_dates = [str(close[i]) for i in range(100)]

        self.check_error_raised(
            lambda: plotter.candlestick(open, high, low, close, bad_dates),
            ValueError,
            "Size of dates: 100 is different from the data size: 149"
        )

        plotter.candlestick(open, high, low, close)

        self.check_error_raised(
            lambda: plotter.line(open[:100]),
            ValueError,
            "ySize: 100 does not match the number of datapoints on the plot 149."
        )

    def check_error_raised(self, func, error, command):
        """
        This is needed
        """
        try:
            func()
            assert False, "func() did not throw an error."
        except error as e:
            assert command in str(e), f"'{command}' not found in '{str(e)}'."


t = TestPlotter()
N = t.N()
candlestick_data = t.candlestick_data(N)

funcs = dir(t)
for name in funcs:
    if name.startswith("test_"):
        method = getattr(t, name)  # get the actual bound method
        print(f"Running {name}...")
        method(candlestick_data)
