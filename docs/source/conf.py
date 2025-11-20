# -- Path setup --------------------------------------------------------------
import os
import sys
from datetime import date
from pathlib import Path

root = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(root / "src"))

# -- Project information -----------------------------------------------------

project = "rallyplot"
author = "rallyplot"
copyright = f"{date.today().year}, {author}"
try:
    from importlib.metadata import version as get_version
    release = get_version("rallyplot")
except Exception:
    release = "0.0.0"

version = ".".join(release.split(".")[:2])


# -- General configuration ---------------------------------------------------

autodoc_mock_imports = [
    "numpy", "pandas"
]

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.napoleon",
    "sphinx.ext.intersphinx",
    "sphinx.ext.viewcode",
    "sphinx.ext.todo",
    "sphinx.ext.mathjax",
    "myst_parser",
    "sphinx_design",
    "sphinx_autodoc_typehints",
    "numpydoc",
    "nbsphinx",
    "breathe",
    "sphinx.ext.githubpages",
]

breathe_projects = {
    "rallyplot": os.path.join(
        os.path.dirname(__file__), "cpp", "xml"
    ),
}
breathe_default_project = "rallyplot"

# Optional: auto-pick Sphinx domains by file extension
breathe_domain_by_extension = {
    "h": "cpp",
    "cpp": "cpp",
}
# Optional defaults for member visibility
breathe_default_members = ("members", "undoc-members")



# Automatically generate stub pages for API
autosummary_generate = True
numpydoc_class_members_toctree = False  # stops stubs warning
#toc_object_entries_show_parents = "all"

autodoc_default_options = {
    'members': True,
    "member-order": "bysource",
    'special-members': False,
    'private-members': False,
    'inherited-members': False,
    'undoc-members': True,
    'exclude-members': "",
}



templates_path = ["_templates"]
exclude_patterns = ["build", "Thumbs.db", ".DS_Store"]

autodoc_typehints = "description"

# -- Intersphinx -------------------------------------------------------------

#intersphinx_mapping = {
#   "python": ("https://docs.python.org/3", {}),
#    "numpy": ("https://numpy.org/doc/stable/", {}),
#    "pandas": ("https://pandas.pydata.org/docs/", {}),
#}

# -- HTML output with PyData Sphinx Theme ------------------------------------

html_show_sourcelink = False  # disables "Show Source" link

html_theme = "pydata_sphinx_theme"

html_theme_options = {
    "logo": {
        "text": project,
        # "image_light": "_static/logo-light.png",
        # "image_dark": "_static/logo-dark.png",
    },
    "navbar_end": ["theme-switcher",  "navbar-icon-links"],
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/rallyplot/rallyplot",
            "icon": "fab fa-github",
        },
    ],
    "use_edit_page_button": False,   # turn off "Edit on GitHub"
    "show_nav_level": 2,
    "show_toc_level": 2,             # no local TOC in sidebar
    "show_prev_next": False,         # remove prev/next at bottom
  #  "secondary_sidebar_items": {},   # removes right sidebar entirely
    "pygments_light_style": "a11y-high-contrast-light",
    "pygments_dark_style": "a11y-high-contrast-dark",
}

html_context = {
    "github_user": "rallyplot",
    "github_repo": "rallyplot",
    "github_version": "main",
    "doc_path": "docs",
    "default_mode": "auto",
}

html_static_path = ["_static"]

# -- MyST (Markdown) ---------------------------------------------------------
myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "dollarmath",
    "tasklist",
]
