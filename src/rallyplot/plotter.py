from __future__ import annotations

import numpy as np

from typing import Literal, Union
from datetime import datetime

import pandas as pd
from datetime import timezone
from pathlib import Path
import os
import warnings
import sys
import platform
import copy
import numbers

# Do not import pythonBindings if we are building docs.
# This way we don't have to build the whole lib just to build the docs.
BUILDING_DOCS = os.getenv("RALLYPLOT_BUILDING_DOCS", False)

if not BUILDING_DOCS:
    from . import pythonBindings

# -------------------------------------------------------------------------------------
# Argument Types
# -------------------------------------------------------------------------------------

FontType = Literal["consola", "arial"]

Array = Union[
    list, tuple, np.ndarray
]

Dates = Union[
    list[str], list[datetime], pd.Series, pd.DatetimeIndex
]

FontWeightType = Literal[
    "thin",
    "extralight",
    "light",
    "normal",
    "regular",
    "medium",
    "demibold",
    "bold",
    "extrabold",
    "black"
]
CandlestickMode = Literal[
    "full",
    "no_caps",
    "body_only",
    "line_open",
    "line_closed"
]
ScatterShapeType = Literal[
    "circle",
    "triangle_up",
    "triangle_down",
    "cross"
]
HoverValueDisplayMode = Literal[
    "always_show",
    "only_under_mouse",
    "off"
]


def get_toy_candlestick_data(N: int = 100_000, seed: int = None):
    """"""
    data = pythonBindings.get_toy_candlestick_data(
        N=N, seed=seed
    )

    volume = data.pop("volume")
    dates = data.pop("dates")

    data_df = pd.DataFrame(data)

    return data_df, volume, dates


# -------------------------------------------------------------------------------------
# Plotter
# -------------------------------------------------------------------------------------

class Plotter:

    def __init__(
        self,
        width: int = 800,
        height: int = 600,
        color_mode: Literal["light", "dark"] = "light",
        anti_aliasing_samples: int = 0,
        axis_tick_label_font: FontType = "arial",
        axis_tick_label_font_size: int = 12,
        axis_right: bool = True,
        width_margin_size: int = 50,
        height_margin_size: int = 25
    ):
        """ The Plotter class controls all plotting.

        Parameters
        ----------

        width
            Width of the figure window.
        height
            Height of the figure window.
        color_mode
            "light" or "dark" mode.
        anti_aliasing_samples
            Number of samples used in antialiasing smoothing. Zero is off, gives best performance.
            Higher looks smoother but is slower. Flickering of thin-line plots may occur when less than 4.*/
        axis_tick_label_font
            Font to use for axis tick labels.
        axis_tick_label_font_size
            Font size for the axis tick labels.
        axis_right
            If `true`, y-axis is located on the right of the plot. Otherwise, left of the plot.
        width_margin_size
            Size of margin between y-axis and the edge of the figure.
        height_margin_size
            Size of the margin between the x-axis and the bottom edge of the figure.

        """
        # Patch the QT_PLUGIN_PATH to use our vendored plugins. This only needs to
        # be set while the pythonBindings are initialised, then the path is
        # held on the cpp side. Return the path to its original state to not
        # interfere with other packages.
        orig_paths = copy.deepcopy(os.environ.get("QT_PLUGIN_PATH", None))
        qt_build_dir = Path(__file__).parent.as_posix()
        os.environ["QT_PLUGIN_PATH"] = os.path.join(qt_build_dir, "plugins")

        self._plotter = pythonBindings.Plotter(
            width=width,
            height=height,
            color_mode=color_mode,
            anti_aliasing_samples=anti_aliasing_samples,
            axis_tick_label_font=axis_tick_label_font,
            axis_tick_label_font_size=axis_tick_label_font_size,
            axis_right=axis_right,
            width_margin_size=width_margin_size,
            height_margin_size=height_margin_size
        )

        if orig_paths:
            os.environ["QT_PLUGIN_PATH"] = orig_paths
        else:
            os.environ.pop("QT_PLUGIN_PATH")

    def _grab_frame_buffer(self, row, col):
        return self._plotter._grab_frame_buffer(row, col)

    def start(self):
        """Start the event loop to display and interact with plots. """
        self._plotter.start()

    def finish(self):
        """Ensure c++ side is completely cleared.

        This ensures you can the below and avoid seg faults.:
        plotter = Plotter()
        plotter.finish()
        plotter = Plotter()
        """
        del self._plotter
        self._plotter = None

    def set_background_color(self, color: Array):
        """Set the background color for the subplot.

        color
            Array of length 1 to 4 setting RGBA color values.
            Missing values are set to 0.0f for color and 1.0f for alpha.
            e.g. {0.1f, 0.2f} sets {0.1f, 0.2f, 0.0f, 1.0f}.
        """
        self._plotter.set_background_color(self._to_list(color))

    def set_camera_settings(
        self,
        key_zoom_speed: float = 2.5,
        mouse_zoom_speed: float = 0.004,
        key_pan_speed: float = 0.35,
        mouse_pan_speed: float = 0.70,
        wheel_speed: float = 0.002,
        lock_most_recent_date: bool = False,
        fix_zoom_at_edge: bool = False
    ):
        """Set the camera settings that control pan and zooming.

        Parameters
        ----------
        key_zoom_speed
            Controls the speed of zoom by key-press (Q, E, Z, C).
        mouse_zoom_speed
            Controls the speed of zoom by mouse move (right-click).
        key_pan_speed
            Controls the speed of panning by key-press (A, D, W, X).
        mouse_pan_speed
            Controls the speed of panning by mouse move (left-click).
        wheel_speed
            Controls the speed of zoom by the mouse wheel.
        lock_most_recent_date
            If `true`, the axis zoom / pan is locked to the largest x-axis position.
        fix_zoom_at_edge
            All zooming will be fixed to the y-axis (right or left depending on PlotterArgs.axisRight).

        """
        self._plotter.set_camera_settings(
            key_zoom_speed=key_zoom_speed,
            mouse_zoom_speed=mouse_zoom_speed,
            key_pan_speed=key_pan_speed,
            mouse_pan_speed=mouse_pan_speed,
            wheel_speed=wheel_speed,
            lock_most_recent_date=lock_most_recent_date,
            fix_zoom_at_edge=fix_zoom_at_edge
        )

    def set_crosshair_settings(
        self,
        on: bool = True,
        font: FontType = "arial",
        font_size: int = 10,
        linewidth: float = 1.0,
        line_color: Array | None = None,
        background_color: Array | None = None,
        font_color: Array | None = None,
    ):
        """Settings for the crosshair that follows the mouse over the plot.

        Parameters
        ----------
        on
            If `False`, the crosshair is turned off.
        font
            Font of the label that appears in the axis.
        font_size
            Font size of the label that appears in the axis.
        linewidth
            Crosshair line width.
        line_color
            Crosshair line color (array-like, length 1-4, RGBA). Default is based on ColorMode.
        background_color
            Background color (array-like, length 1-4, RGBA) of the label that appears in the axis. Default is based on ColorMode.
        font_color
            Color (array-like, length 1-4, RGBA) of the font that appears in the axis. Default is based on ColorMode.
        """
        self._plotter.set_crosshair_settings(
            on=on,
            font=font,
            font_size=font_size,
            linewidth=linewidth,
            line_color=self._to_list(line_color),
            background_color=self._to_list(background_color),
            font_color=self._to_list(font_color)
        )

    def set_draw_line_settings(
        self,
        linewidth: float = 0.25,
        color: Array | None = None,
    ):
        """Control how lines drawn on the plot are displayed.

        Parameters
        ----------
        linewidth
            Width of the drawn line.
        color
            Color (array-like, length 1-4, RGBA) of the drawn line. Default blue.
        """
        self._plotter.set_draw_line_settings(
            linewidth=linewidth,
            color=self._to_list(color)
        )

    def set_hover_value_settings(
        self,
        display_mode: HoverValueDisplayMode = "always_show",
        font: FontType = "arial",
        font_size: int = 10,
        font_color: Array | None = None,
        background_color: Array | None = None,
        border_color: Array | None = None
    ):
        """Control how the pop-up label that appears on mouse hover over plot is displayed.

        Parameters
        ----------
        display_mode
            "always_show" will always display the pop-up. "only_under_mouse" will show only when mouse
            is hovered over the plot. Otherwise, "off".
        font
            Font of the hover label.
        font_size
            Font size of the hover label.
        font_color
            Font color (array-like, length 1-4, RGBA) of the hover label. Default is based on ColorMode.
        background_color
            Background color (array-like, length 1-4, RGBA) of the hover label. Default is based on ColorMode.
        border_color
            Border color (array-like, length 1-4, RGBA) of the hover label. Default is based on ColorMode.
        """
        self._plotter.set_hover_value_settings(
            display_mode=display_mode,
            font=font,
            font_size=font_size,
            font_color=self._to_list(font_color),
            background_color=self._to_list(background_color),
            border_color=self._to_list(border_color)
        )

    def set_x_axis_settings(
        self,
        min_num_ticks: int = 6,
        max_num_ticks: int = 12,
        init_num_ticks: int = 8,
        show_ticks: bool = True,
        tick_linewidth: float = 1.0,
        tick_size: float = 0.025,
        show_gridline: bool = True,
        gridline_width: float = 0.4,
        axis_linewidth: float = 1.0,
        gridline_color: Array | None = None,
        axis_color: Array | None = None,
        font_color: Array | None = None,
        linked_subplot_idx:  int | None = None,
    ):
        """Control how the x-axis is displayed.

        Parameters
        ----------
        min_num_ticks
            Minimum number of ticks that should be displayed. Should be a multiple of 2.
            If too close to max_num_ticks, a flickering artifact may be observed.
        max_num_ticks
            Maximum number of ticks that should be displayed. Should be a multiple of 2.
            If too close to min_num_ticks, a flickering artifact may be observed.
        init_num_ticks
            Initial number of ticks, must be a multiple of two and between min / max num ticks.
        show_ticks
            If `true`, small tick-lines will be displayed.
        tick_linewidth
            Width of the tick line. May have no effect on some GPUs.
        tick_size
            Length of the tick line.
        show_gridline
            If `true`, grid lines (that extend the tick line across the plot) are displayed.
        gridline_width
            Width of the grid line. May have no effect on some GPUs.
        axis_linewidth
            Width of the axis line. May have no effect on some GPUs.
        gridline_color
            Grid line (array-like, length 1-4, RGBA) color, if `None` uses the default according to ColorMode.
        axis_color
            Axis color (array-like, length 1-4, RGBA) , if `None` uses the default according to ColorMode.
        font_color
            Axis tick label color(array-like, length 1-4, RGBA), if `None` uses the default according to ColorMode.
        linked_subplot_idx
            The linked subplot on which to set the settings. If `None`, will be applied to all linked subplots.
        """
        self._plotter.set_x_axis_settings(
            min_num_ticks=min_num_ticks,
            max_num_ticks=max_num_ticks,
            init_num_ticks=init_num_ticks,
            show_ticks=show_ticks,
            tick_linewidth=tick_linewidth,
            tick_size=tick_size,
            show_gridline=show_gridline,
            gridline_width=gridline_width,
            axis_linewidth=axis_linewidth,
            gridline_color=self._to_list(gridline_color),
            axis_color=self._to_list(axis_color),
            font_color=self._to_list(font_color),
            linked_subplot_idx=linked_subplot_idx
        )

    def set_y_axis_settings(
        self,
        min_num_ticks: int = 6,
        max_num_ticks: int = 12,
        init_num_ticks: int = 12,
        show_ticks: bool = False,
        tick_linewidth: float = 1.0,
        tick_size: float = 0.025,
        show_gridline: bool = True,
        gridline_width: float = 0.4,
        axis_linewidth: float = 1.0,
        tick_label_decimal_places: int = 2,
        gridline_color: Array | None = None,
        axis_color: Array | None = None,
        font_color: Array | None = None,
        linked_subplot_idx:  int | None = None,
    ):
        """
        Control how the y-axis is displayed.

        Parameters
        ----------
        min_num_ticks
            Minimum number of ticks that should be displayed. Should be a multiple of 2.
            If too close to max_num_ticks, a flickering artifact may be observed.
        max_num_ticks
            Maximum number of ticks that should be displayed. Should be a multiple of 2.
            If too close to min_num_ticks, a flickering artifact may be observed.
        init_num_ticks
            Initial number of ticks, must be a multiple of two and between min / max num ticks.
        show_ticks
            If `true`, small tick-lines will be displayed.
        tick_linewidth
            Width of the tick line. May have no effect on some GPUs.
        tick_size
            Length of the tick line.
        show_gridline
            If `true`, grid lines (that extend the tick line across the plot) are displayed.
        gridline_width
            Width of the grid line. May have no effect on some GPUs.
        axis_linewidth
            Width of the axis line. May have no effect on some GPUs.
        tick_label_decimal_places
            Controls the number of decimal places for the y-axis tick labels.
        gridline_color
            Grid line color, if `None` uses the default according to ColorMode.
        axis_color
            Axis color (array-like, length 1-4, RGBA) , if `None` uses the default according to ColorMode.
        font_color
            Axis tick label color (array-like, length 1-4, RGBA) , if `None` uses the default according to ColorMode.
        linked_subplot_idx
            The linked subplot on which to set the settings. If `None`, will be applied to all linked subplots.
        """
        self._plotter.set_y_axis_settings(
            min_num_ticks=min_num_ticks,
            max_num_ticks=max_num_ticks,
            init_num_ticks=init_num_ticks,
            show_ticks=show_ticks,
            tick_linewidth=tick_linewidth,
            tick_size=tick_size,
            show_gridline=show_gridline,
            gridline_width=gridline_width,
            axis_linewidth=axis_linewidth,
            tick_label_decimal_places=tick_label_decimal_places,
            gridline_color=self._to_list(gridline_color),
            axis_color=self._to_list(axis_color),
            font_color=self._to_list(font_color),
            linked_subplot_idx=linked_subplot_idx
        )

    def pin_y_axis(self, on: bool = True, linked_subplot_idx: int | None = None):
        """Fix the y-axis zoom to the minimum and maximum values of the plot currently in view.

        Parameters
        ----------
        on
            If `False`, y-axis is unpinned and can be zoomed freely.
        linked_subplot_idx
            The linked subplot to pin. If `None`, will be applied to all linked subplots.
        """
        self._plotter.pin_y_axis(on=on, linked_subplot_idx=linked_subplot_idx)

    def set_y_limits(self, min: float | None = None, max: float | None = None, linked_subplot_idx: int | None = None):
        """Set minimum and / or maximum values for the y-axis.

        Only has an effect if pin_y_axis() is set to `False`.

        Parameters
        ----------
        min
            Minimum value for the y-axis, if `None` is set to -inf.
        max
            Maximum value for the y-axis, if `None` is set to inf.
        linked_subplot_idx
            The linked subplot on which to limit the y-axis view. If `None`, will be applied to all linked subplots.
        """
        self._plotter.set_y_limits(min=min, max=max, linked_subplot_idx=linked_subplot_idx)

    def set_x_limits(self, min: int | str | datetime | None, max: int | str | datetime | None):
        """Set the minimum and maximum values for the x-axis.

        Must be the same type as the x-axis data. x-axis values should be unique.

        Parameters
        ----------
        min
            Minimum value for the x-axis, if `None` is set to the minimum x value.
            Must be the same type as `dates` used for the plot.
        max
            Maximum value for the x-axis, if `None` is set to the maximum x value.
            Must be the same type as `dates` used for the plot.
        """
        self._plotter.set_x_limits(min=min, max=max)

    def link_y_axes(self, on: bool = True):
        """
        Link the y-axis across all linked subplots.

        on
            If `true`, zooming on one plot will zoom on all linked subplots.
        """
        self._plotter.link_y_axes(on=on)

    def set_y_label(
        self,
        text: str,
        font: FontType = "arial",
        weight: FontWeightType = "bold",
        font_size: int = 12,
        color: Array | None = None
    ):
        """Set a label on the y-axis for the subplot.

        Parameters
        ----------
        text
            Text to set as the label.
        font
            Font of the label.
        weight
            Weight of the label font.
        font_size
            Point size of the label font.
        color
            Color (array-like, length 1-4, RGBA) of the label text.
        """
        self._plotter.set_y_label(
            text=text,
            font=font,
            weight=weight,
            font_size=font_size,
            color=self._to_list(color)
        )

    def set_x_label(
        self,
        text: str,
        font: FontType = "arial",
        weight: FontWeightType = "bold",
        font_size: int = 12,
        color: Array | None = None
    ):
        """
        Set a label on the x-axis for the subplot.

        Parameters
        ----------
        text
            Text to set as the label.
        font
            Font of the label.
        weight
            Weight of the label font.
        font_size
            Point size of the label font.
        color
            Color (array-like, length 1-4, RGBA) of the label text.
        """
        self._plotter.set_x_label(
            text=text,
            font=font,
            weight=weight,
            font_size=font_size,
            color=self._to_list(color)
        )

    def set_title(
        self,
        text: str,
        font: FontType = "arial",
        weight: FontWeightType = "extrabold",
        font_size: int = 12,
        color: Array | None = None,
    ):
        """Set a title on the subplot.

        Parameters
        ----------
        text
            Text to set as the title.
        font
            Font of the title.
        weight
            Weight of the title font.
        font_size
            Point size of the title font.
        color
            Color (array-like, length 1-4, RGBA) of the title text
        """
        self._plotter.set_title(
            text=text,
            font=font,
            weight=weight,
            font_size=font_size,
            color=self._to_list(color)
        )

    def set_legend(
            self,
            label_names: list[str],
            linked_subplot_idx: int = -1,
            legend_size_scalar: float = 1.0,
            x_box_pad: float = 10.0,
            y_box_pad: float = 2.0,
            line_width:  float = 35.0,
            line_height: float = 7.5,
            x_line_pad_left: float = 5.0,
            x_line_pad_right: float = 5.0,
            x_text_pad_right: float = 2.0,
            y_inter_item_pad: float = 5.0,
            y_item_pad: float = 2.0,
            font: FontType = "arial",
            font_size: int = 15,
            font_color: Array | None = None,
            box_color: Array = (1.0, 1.0, 1.0, 0.0),
    ):
        """Set a legend on a linked subplot.

        Parameters
        ----------
        label_names
            A list of label names for the legend. Must be the same length
            as the number of plots in the linked subplot.
        linked_subplot_idx
            Linked subplot on which to add the legend.
        legend_size_scalar
            A convenient scalar by which to increase or decrease the legend size.
            Other settings add further customization to the legend size.
        x_box_pad
            X position of the legend expressed as distance from the right of the plot.
        y_box_pad
            Y position of the legend expressed as distance from the top of the plot.
        line_width
            Width of the legend item colored lines.
        line_height
            Height of the legend items colored lines.
        x_line_pad_left
            Padding between the edge of the legend box and the text (left)
        x_line_pad_right
            Padding between the legend item colored line (left) i.e. between the line text.
        x_text_pad_right
            Padding between the text (right) i.e. the text and the item colored line.
        y_inter_item_pad
            Vertical padding between legend items.
        y_item_pad
            Padding between the legend items (top and bottom) and the legend box.
        font
            Font for the legend item text.
        font_size
            Font size for the legend item text.
        font_color
            Font color (array-like, length 1-4, RGBA) for the legend item text.
        box_color
            Background color (array-like, length 1-4, RGBA) for the legend box.
        """
        string_vector_labels = pythonBindings.StringVector(label_names)

        self._plotter.set_legend(
            label_names=string_vector_labels,
            linked_subplot_idx=linked_subplot_idx,
            legend_size_scalar=legend_size_scalar,
            x_box_pad=x_box_pad,
            y_box_pad=y_box_pad,
            line_width=line_width,
            line_height=line_height,
            x_line_pad_left=x_line_pad_left,
            x_line_pad_right=x_line_pad_right,
            x_text_pad_right=x_text_pad_right,
            y_inter_item_pad=y_inter_item_pad,
            y_item_pad=y_item_pad,
            font=font,
            font_size=font_size,
            font_color=self._to_list(font_color),
            box_color=self._to_list(box_color)
        )

    def add_subplot(
        self,
        row: int,
        col: int,
        row_span: int,
        col_span: int
    ):
        """
        Add a new subplot to the figure.

        After adding, all calls on Plotter will operate on the added subplot.
        To change, use set_active_subplot.

        Specify the row and col index at which to add the subplot, as well as
        how many rows and columns it spans. For example, to add a second subplot
        when there is 1 existing plot:
        (0, 1, 1, 1) will add a new subplot to the right of the existing plot.
        (1, 0, 1, 1) will add the new subplot underneath the existing plot.

        Note that a 'subplot' is a different entity to a 'linked subplot'.
        All linked subplots share the x-axis, while a subplot is a completely
        separate set of plots.

        Parameters
        ----------
        row
            Index of the row in which to add the subplot.
        col
            Index of the column in which to add the subplot.
        row_span
            Number of rows the subplot will span.
        col_span
            Number of columns the subplot will span.
        """
        self._plotter.add_subplot(
            row=row,
            col=col,
            row_span=row_span,
            col_span=col_span
        )

    def set_active_subplot(
        self,
        row: int,
        col: int
    ):
        """Set the 'active' subplot to the subplot at index row, col.

        All calls to Plotter e.g. Plotter.candlestick() operate on the active subplot.
        Row, Col should be the top-left corner of the subplot in the case it is spanned.

        Parameters
        ----------
        row
            Row of the subplot to set active
        col
            Column of the subplot to set active
        """
        self._plotter.set_active_subplot(
            row=row,
            col=col
        )

    def resize_linked_subplots(
        self,
        y_heights: Array
    ):
        """Resize the linked subplots.

        Parameters
        ----------
        y_heights
            A vector of height proportions. Length must equal the
            number of linked subplots on the plot and must sum to 1.
        """
        self._plotter.resize_linked_subplots(
            list(y_heights)
        )

    def add_linked_subplot(self, height_as_proportion: float):
        """Add a linked subplot to the subplot.

        A 'linked subplot' is a subplot that shares the x-axis with the current plot.
        This is in contrast to a 'subplot' which is an entirely separate plot.

        Parameters
        ----------
        height_as_proportion
            The height of the new linked subplot as a proportion of the plot (in range [0, 1]).
        """
        self._plotter.add_linked_subplot(height_as_proportion)

    # Plotting
    # ---------------------------------------------------------------------------------

    def candlestick_from_df(
        self,
        df: pd.DataFrame,
        dates: Dates | None = None,
        linked_subplot_idx: int = -1,
        up_color: Array = (0.0314, 0.6, 0.506, 1.0),
        down_color: Array = (0.957, 0.204, 0.266, 1.0),
        mode: CandlestickMode = "no_caps",
        candle_width_ratio: float = 0.75,
        cap_width_ratio: float = 0.5,
        line_mode_linewidth: float = 1.0,
        line_mode_miter_limit: float = 3.0,
        line_mode_basic_line: bool = False
    ):
        """
        Add a candlestick plot to the linked subplot.

        Parameters
        ----------
        df
            A Pandas dataframe either columns "Open", "High", "Low", "Close"  (all lowercase accepted e.g. "open").
        dates
            A list of string (labels) or datetime (must be UTC) to use as x-axis labels. If `None`, index will be displayed.
            If pd.Series, it will be converted to a list internally.
        linked_subplot_idx
           The index of the linked subplot on which to plot the candlesticks. By default, it is the most recently added linked subplot.
        up_color
            Color (array-like, length 1-4, RGBA) for candles when close price is higher than open price.
        down_color
            Color (array-like, length 1-4, RGBA) for candles when open price is lower than close price.
        mode
            Control how candles are displayed (see CandlestickMode).
        candle_width_ratio
            Ratio between candle and gap width, a float between (0, 1] e.g. 1 is no space between candles.
        cap_width_ratio
            Ratio between candle and cap width, a double between (0, 1] e.g. 1 the cap is the width of the candle.
        line_mode_linewidth
            Line width for open-line and close-line mode for the candlestick plot.
        line_mode_miter_limit
            Miter limit controls the maximum line-segment connection length, for open-line and close-line mode.
        line_mode_basic_line
            If `true`, a simple line plot with fixd width is used (`width` and `miterLimit` have no effect). This is much faster.
        """
        # Existence check
        lower_cols = pd.Index(map(str, df.columns)).str.lower()
        for accepted_name in ["open", "high", "low", "close"]:
            if accepted_name not in lower_cols:
                raise ValueError(f"{accepted_name} (or {accepted_name.title()}) not found in the passed dataframe.")

        # Map lowercase -> Series
        col_map = {c.lower(): df[c] for c in df.columns}

        self.candlestick(
            open=col_map["open"].to_numpy(),
            high=col_map["high"].to_numpy(),
            low=col_map["low"].to_numpy(),
            close=col_map["close"].to_numpy(),
            dates=dates,
            linked_subplot_idx=linked_subplot_idx,
            up_color=up_color,
            down_color=down_color,
            mode=mode,
            candle_width_ratio=candle_width_ratio,
            cap_width_ratio=cap_width_ratio,
            line_mode_linewidth=line_mode_linewidth,
            line_mode_miter_limit=line_mode_miter_limit,
            line_mode_basic_line=line_mode_basic_line,
        )


    def candlestick(
        self,
        open: np.ndarray,
        high: np.ndarray,
        low: np.ndarray,
        close: np.ndarray,
        dates: Dates | None = None,
        linked_subplot_idx: int = -1,
        up_color: Array = (0.0314, 0.6, 0.506, 1.0),
        down_color: Array = (0.957, 0.204, 0.266, 1.0),
        mode: CandlestickMode = "no_caps",
        candle_width_ratio: float = 0.75,
        cap_width_ratio: float = 0.5,
        line_mode_linewidth: float = 1.0,
        line_mode_miter_limit: float = 3.0,
        line_mode_basic_line: bool = False
    ):
        """
        Add a candlestick plot to the linked subplot.

        Parameters
        ----------
        open
           Vector of candle open prices.
        high
           Vector of candle high prices.
        low
           Vector of candle low prices.
        close
           Vector of candle close prices.
        dates
            A list of string (labels) or datetime (must be UTC) to use as x-axis labels. If `None`, index will be displayed.
            If pd.Series, it will be converted to a list internally.
        linked_subplot_idx
           The index of the linked subplot on which to plot the candlesticks. By default, it is the most recently added linked subplot.
        up_color
            Color (array-like, length 1-4, RGBA) for candles when close price is higher than open price.
        down_color
            Color (array-like, length 1-4, RGBA) for candles when open price is lower than close price.
        mode
            Control how candles are displayed (see CandlestickMode).
        candle_width_ratio
            Ratio between candle and gap width, a float between (0, 1] e.g. 1 is no space between candles.
        cap_width_ratio
            Ratio between candle and cap width, a double between (0, 1] e.g. 1 the cap is the width of the candle.
        line_mode_linewidth
            Line width for open-line and close-line mode for the candlestick plot.
        line_mode_miter_limit
            Miter limit controls the maximum line-segment connection length, for open-line and close-line mode.
        line_mode_basic_line
            If `true`, a simple line plot with fixd width is used (`width` and `miterLimit` have no effect). This is much faster.
        """
        if isinstance(open, pd.DataFrame):
            raise ValueError("Use `candlestick_from_df` to pass a Pandas DataFrame.")

        open = self._handle_data_array(open)
        high = self._handle_data_array(high)
        low = self._handle_data_array(low)
        close = self._handle_data_array(close)

        if dates is not None:
            dates : pythonBindings.DatetimeVector | pythonBindings.StringVector
            dates = self._check_and_process_dates(dates)

        self._plotter.candlestick(
            open=open,
            high=high,
            low=low,
            close=close,
            dates=dates,
            linked_subplot_idx=linked_subplot_idx,
            up_color=self._to_list(up_color),
            down_color=self._to_list(down_color),
            mode=mode,
            candle_width_ratio=candle_width_ratio,
            cap_width_ratio=cap_width_ratio,
            line_mode_linewidth=line_mode_linewidth,
            line_mode_miter_limit=line_mode_miter_limit,
            line_mode_basic_line=line_mode_basic_line
        )

    def line(
        self,
        y: np.ndarray | pd.Series,
        dates: Dates | None = None,
        linked_subplot_idx: int = -1,
        color: Array | None = (0.5, 0.5, 0.5, 1.0),
        width: float = 0.5,
        miter_limit: float = 3.0,
        basic_line: bool = False
    ):
        """
        Add a line plot to the linked subplot.

        Parameters
        ----------
        y
            Numpy array of float datapoints to plot.
        dates
            A list of string (labels) or datetime (must be UTC) to use as x-axis labels. If `None`, index will be displayed.
            If pd.Series, it will be converted to a list internally.
        linked_subplot_idx
            The index of the linked subplot on which to plot the line plot. By default, it is the most recently added linked subplot.
        color
            Color (array-like, length 1-4, RGBA) of the line plot.
        width
            Line width for the plot.
        miter_limit
            Miter limit controls the maximum line-segment connection length, for open-line and close-line mode.
        basic_line
            If `true`, a simple line plot with fixd width is used (`width` and `miterLimit` have no effect). This is much faster.
        """
        y = self._handle_data_array(y)

        if dates is not None:
            dates : pythonBindings.DatetimeVector | pythonBindings.StringVector
            dates = self._check_and_process_dates(dates)

        self._plotter.line(
            y=y,
            dates=dates,
            linked_subplot_idx=linked_subplot_idx,
            color=self._to_list(color),
            width=width,
            miter_limit=miter_limit,
            basic_line=basic_line
        )

    def bar(
        self,
        y: np.ndarray | pd.Series,
        dates: Dates | None = None,
        linked_subplot_idx: int = -1,
        color: Array | None = (0.03137, 0.6, 0.50588, 0.75),
        width_ratio: float = 0.5,
        min_value: float | None = None
    ):
        """
        Add a bar plot to the linked subplot.

        Parameters
        ----------
        y
            Vector of float datapoints to plot.
        dates
            A list of string (labels) or datetime (must be UTC) to use as x-axis labels. If `None`, index will be displayed.
            If pd.Series, it will be converted to a list internally.
        linked_subplot_idx
            The index of the linked subplot on which to plot the bar plot. By default, it is the most  recently added linked subplot.
        color
            Color (array-like, length 1-4, RGBA) of the bar plot.
        width_ratio
            Ratio between the bar width and inter-bar gap, a float between (0, 1] e.g. 1 is no space between bars.
        min_value
            Minimum value of the bar plot. By default, the minimum number in `y` minus 1% of max y - min y as padding.
        """
        y = self._handle_data_array(y)

        if dates is not None:
            dates : pythonBindings.DatetimeVector | pythonBindings.StringVector
            dates = self._check_and_process_dates(dates)

        self._plotter.bar(
            y=y,
            dates=dates,
            linked_subplot_idx=linked_subplot_idx,
            color=self._to_list(color),
            width_ratio=width_ratio,
            min_value=min_value
        )

    def scatter(
        self,
        x: np.ndarray | Dates,
        y: np.ndarray | pd.Series,
        linked_subplot_idx: int = -1,
        shape: ScatterShapeType = "circle",
        color: Array = (0.12, 0.46, 0.70, 1.0),
        fixed_size: bool = True,
        marker_size_fixed: float = 0.025,
        marker_size_free: float = 10.0
    ):
        """
        Add a scatter plot to the linked subplot.

        Parameters
        ----------
        x
            A numpy array of indexes, or list of string or datetimes (UTC) of x-axis position of the scatter points.
            The type must match the current x tick label type.
        y
            Vector of floats containing y-axis data. Must match x in length.
        linked_subplot_idx
            The index of the linked subplot on which to plot the scatter plot. By default, it is the most recently added linked subplot.
        shape
            Shape of the scatter marker (see ScatterShapeType)
        color
            Color (array-like, length 1-4, RGBA) of the scatter marker.
        fixed_size
            If `true`, size of the scatter marker is the same at all zooms.
            If `False`, the size of the marker will decrease when zoomed out.
        marker_size_fixed
            Size of the scatter marker when `fixed_size` is `true`.
        marker_size_free
            Size of the scatter marker when `fixed_size` if `False`.
        """
        if isinstance(x, pd.Series):
            if pd.api.types.is_numeric_dtype(x):
                x = x.to_numpy()

        y = self._handle_data_array(y)

        if not (isinstance(x, np.ndarray) and np.issubdtype(x.dtype, np.integer)):
            x_processed = self._check_and_process_dates(x)
        else:
            x_processed = x
            self._check_numpy_arrays(x)

        self._check_numpy_arrays(y)

        self._plotter.scatter(
            x=x_processed,
            y=y,
            linked_subplot_idx=linked_subplot_idx,
            shape=shape,
            color=self._to_list(color),
            fixed_size=fixed_size,
            marker_size_fixed=marker_size_fixed,
            marker_size_free=marker_size_free
        )

    # Helpers
    # ---------------------------------------------------------------------------------

    def _check_and_process_dates(self, dates: None | list[str] | list[datetime]):
        """Handle a list of dates.

        Return a processed list of dates. Under the hood these are transferred to the C++
        side with Pybind11, so must be converted to special C++-side vectors. This
        requires a copy operation (numpy arrays used for actual data are not copied).
        """
        if dates is None:
            return None

        if isinstance(dates, np.ndarray):
            dates = dates.tolist()

        if isinstance(dates, pd.Series):
            if pd.api.types.is_datetime64_any_dtype(dates):
                dates = dates.dt.to_pydatetime().tolist()
            else:
                dates = dates.astype(str).tolist()

        if isinstance(dates[0], datetime):
            if dates[0].tzinfo != timezone.utc:
                raise ValueError("`dates` must be UTC datetime.")

            # this is a copy operation
            dates_formatted = pythonBindings.DatetimeVector(dates)
            return dates_formatted

        if isinstance(dates[0], str):
            # this is a copy operation
            dates_formatted = pythonBindings.StringVector(dates)
            return dates_formatted

        else:
            raise ValueError("`dates` must be list of string labels or UTC datetimes.")

    def _to_list(self, optional_array: Array | None):
        """Convert an array-like object to list or return `None` if `None`

        While an array-like object is assumed, the edge-case where a single
        numeric value is passed, as in `color` is handled.
        """
        if optional_array is not None:
            try:
                list_array = list(optional_array)
            except TypeError:
                assert self.is_number(optional_array), "Value to convert to list must be iterative or numeric."
                list_array = [optional_array]
        else:
            list_array = None

        return list_array

    def is_number(self, x):
        return isinstance(x, numbers.Number) or (
                np.isscalar(x) and np.issubdtype(np.asarray(x).dtype, np.number)
        )

    def _check_numpy_arrays(self, *args):
        """"""
        for array in args:
            assert isinstance(array, np.ndarray)

            if not array.flags['C_CONTIGUOUS']:
                raise ValueError(
                    "All passed numpy arrays must be contiguous in memory."
                    "Run np.ascontiguousarray() on the array before passing."
                )

            if array.ndim != 1:
                raise ValueError(
                    "Only one-dimensional data can be displayed on the plot."
                )

    def _handle_data_array(self, y: pd.Series | list | tuple | np.ndarray):
        """"""
        y = np.asarray(y, dtype=np.float32)

        self._check_numpy_arrays(y)

        return y
