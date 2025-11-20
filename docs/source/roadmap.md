(roadmap)=
# Roadmap

The roadmap contains key priorities for `rallyplot`. Inclusion here is not a guarantee of
completion. Please feel free to add suggestions and give feedback on new features
on our GitHub Issues page.

We wellcome all contributions and if you are interested in working on any of these issues, 
please  do get it touch and feel free to open an PR.

- **Real time plotting:** 
At present all plots at static. A great extension would be real-time rendering,
allowing plots to be linked to an incoming data stream continuously updated as new samples arrive.

- **Floating-point Precision Artefacts:**
When rendering datasets with large x-axis magnitudes (typically around 10 million samples and above), 
visible precision artefacts appear near the rightmost portion of the plot. As GPU supports for 
64-bit floating point operations are limited, use of float32 is unavoidable.
This is a [known issue](https://stackoverflow.com/questions/73657005/lightningchart-js-corruption-of-lineseries-progressive-x) 
in GPU-accelerated plotting. However, there are improvements which could be made when zooming into subsets 
of the data, such as reanchoring the origin.

- **Standalone Scatter Plot:**
Currently the scatter plot must be adding on top of existing plots. It would be nice to allow
standalone scatter plots by automatically generating x-axis values automatically from the subset 
passed in the scatterplot, to allow it to be shown in isolation.
 
- **A fill plot:**
A 'fill' plot that plots a filled region. This could be combined with line 
plot to create fills under a line or between two lines.
 
- **A more flexible API:** 
Currently, all `rallyplot` calls are made through `Plotter()` directly. Central functions
manage subplots, display and camera settings. Another, flexible and possibly more natural
design is to directly expose the underlying plot objects (e.g. plot, subplot, linked subplot). 
It is not clear how useful this will be, but should at least be assessed and cost / benefit analysis performed.

- **Improved Miter Limit:**
On thick line plots, the miter limit is hard to control when zooming and results in
visual artefacts. One solution is to dynamically adjust the miter limit with the zoom.
