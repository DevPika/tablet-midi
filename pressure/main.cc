/* Drawing Area
 *
 * Gtk::DrawingArea is a blank area where you can draw custom displays
 * of various kinds.
 *
 * This demo has two drawing areas. The checkerboard area shows
 * how you can just draw something; all you have to do is write
 * a draw function, as shown here.
 *
 * The "scribble" area is a bit more advanced, and shows how to handle
 * events such as button presses and mouse motion. Click the mouse
 * and drag in the scribble area to draw squiggles. Resize the window
 * to clear the area.
 */

#include <gtkmm.h>
#include <optional>
#include <iostream>

class Example_DrawingArea : public Gtk::Window
{
public:
  Example_DrawingArea();
  ~Example_DrawingArea() override;

protected:
  //draw functions:
  void on_drawingarea_checkerboard_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
  void on_drawingarea_scribble_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);

  //signal handlers:
  void on_drawingarea_scribble_resize(int width, int height);
  // void on_drawingarea_scribble_drag_begin(double start_x, double start_y);
  // void on_drawingarea_scribble_drag_update(double offset_x, double offset_y);
  // void on_drawingarea_scribble_drag_end(double offset_x, double offset_y);
  void on_drawingarea_scribble_stylus_down(double offset_x, double offset_y);
  void on_drawingarea_scribble_stylus_motion(double offset_x, double offset_y);

  void scribble_create_surface();
  void scribble_draw_brush(double x, double y, double pressure);

  //Member widgets:
  Gtk::DrawingArea m_DrawingArea_Scribble;

  Cairo::RefPtr<Cairo::ImageSurface> m_surface;
//   Glib::RefPtr<Gtk::GestureDrag> m_drag;
  Glib::RefPtr<Gtk::GestureStylus> m_stylus;
  double m_start_x = 0.0;
  double m_start_y = 0.0;
};

// //Called by DemoWindow;
// Gtk::Window* do_drawingarea()
// {
//   return new Example_DrawingArea();
// }

Example_DrawingArea::Example_DrawingArea()
{
  set_child(m_DrawingArea_Scribble);
  set_default_size(200, 200);
  // TODO Not working
  // fullscreen();

  // m_DrawingArea_Scribble.set_expand(true);

  // m_drag = Gtk::GestureDrag::create();
  // m_drag->set_button(GDK_BUTTON_PRIMARY);
  // m_DrawingArea_Scribble.add_controller(m_drag);

  m_stylus = Gtk::GestureStylus::create();
  m_stylus->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
  m_DrawingArea_Scribble.add_controller(m_stylus);

  /* set a minimum size */
  m_DrawingArea_Scribble.set_content_width(200);
  m_DrawingArea_Scribble.set_content_height(200);

  m_DrawingArea_Scribble.set_draw_func(
      sigc::mem_fun(*this, &Example_DrawingArea::on_drawingarea_scribble_draw));

  /* Connect signal handlers */
  m_DrawingArea_Scribble.signal_resize().connect(
      sigc::mem_fun(*this, &Example_DrawingArea::on_drawingarea_scribble_resize));

  // m_drag->signal_drag_begin().connect(
  //     sigc::mem_fun(*this, &Example_DrawingArea::on_drawingarea_scribble_drag_begin));
  // m_drag->signal_drag_update().connect(
  //     sigc::mem_fun(*this, &Example_DrawingArea::on_drawingarea_scribble_drag_update));
  // m_drag->signal_drag_end().connect(
  //     sigc::mem_fun(*this, &Example_DrawingArea::on_drawingarea_scribble_drag_end));
  m_stylus->signal_down().connect(
    sigc::mem_fun(*this, &Example_DrawingArea::on_drawingarea_scribble_stylus_down));
  m_stylus->signal_motion().connect(
    sigc::mem_fun(*this, &Example_DrawingArea::on_drawingarea_scribble_stylus_motion));
}

Example_DrawingArea::~Example_DrawingArea()
{
}

void Example_DrawingArea::on_drawingarea_checkerboard_draw(const Cairo::RefPtr<Cairo::Context>& cr,
  int width, int height)
{
  enum { CHECK_SIZE = 10, SPACING = 2 };

  /* At the start of a draw handler, a clip region has been set on
   * the Cairo context, and the contents have been cleared to the
   * widget's background color. The docs for
   * Gdk::Window::begin_paint_region() give more details on how this
   * works.
   */

  int xcount = 0;

  int i = SPACING;
  while (i < width)
  {
    int j = SPACING;
    int ycount = xcount % 2; /* start with even/odd depending on row */
    while (j < height)
    {
      if (ycount % 2)
        cr->set_source_rgb(0.45777, 0, 0.45777);
      else
        cr->set_source_rgb(1, 1, 1);

      /* If we're outside event->area, this will do nothing.
       * It might be mildly more efficient if we handled
       * the clipping ourselves, but again we're feeling lazy.
       */
      cr->rectangle(i, j, CHECK_SIZE, CHECK_SIZE);
      cr->fill();

      j += CHECK_SIZE + SPACING;
      ++ycount;
    }

    i += CHECK_SIZE + SPACING;
    ++xcount;
  }
}

void Example_DrawingArea::on_drawingarea_scribble_draw(const Cairo::RefPtr<Cairo::Context>& cr,
  int, int)
{
  cr->set_source(m_surface, 0, 0);
  cr->paint();
}

// Create a new surface of the appropriate size to store our scribbles.
void Example_DrawingArea::scribble_create_surface()
{
  m_surface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,
    m_DrawingArea_Scribble.get_width(), m_DrawingArea_Scribble.get_height());

  // Initialize the surface to white.
  auto cr = Cairo::Context::create(m_surface);
  cr->set_source_rgb(1, 1, 1);
  cr->paint();
}

void Example_DrawingArea::on_drawingarea_scribble_resize(
  int /* width */, int /* height */)
{
  scribble_create_surface();
}

void Example_DrawingArea::on_drawingarea_scribble_stylus_down(double x, double y) {
  // scribble_draw_brush(x, y, 1);

  // auto cr = Cairo::Context::create(m_surface);
  // cr->begin_new_path();
}

void Example_DrawingArea::on_drawingarea_scribble_stylus_motion(double x, double y) {
  auto numAxes = m_stylus->get_device()->property_n_axes();
  for (auto event : m_stylus->get_backlog()) {
    auto eventPressure = (guint)Gdk::AxisUse::PRESSURE <= numAxes ?
      event.get_value_at_axis((guint)Gdk::AxisUse::PRESSURE) : 1;

    scribble_draw_brush(
      event.get_value_at_axis((guint)Gdk::AxisUse::X),
      event.get_value_at_axis((guint)Gdk::AxisUse::Y),
      eventPressure);
  }

  std::optional<double> pressure = m_stylus->get_axis(Gdk::AxisUse::PRESSURE);
  if (!pressure) {
    scribble_draw_brush(x, y, 1);
  } else {
    scribble_draw_brush(x, y, *pressure);
  }
}

// Draw a rectangle on the screen.
void Example_DrawingArea::scribble_draw_brush(double x, double y, double pressure)
{
  if (!m_surface ||
       m_surface->get_width() != m_DrawingArea_Scribble.get_width() ||
       m_surface->get_height() != m_DrawingArea_Scribble.get_height())
    scribble_create_surface();

  auto size = (int)(pressure * 10);

  const Gdk::Rectangle update_rect((int)x - (int)(size/2), (int)y - (int)(size/2), size, size);

  // Paint to the surface, where we store our state.
  auto cr = Cairo::Context::create(m_surface);
  // cr->set_operator(Cairo::Context::Operator::SATURATE);
  // cr->set_source_rgba(0.0, 0.0, 0.0, 1.0);
  // cr->set_line_width(size);
  // std::cout << pressure << std::endl;
  // cr->line_to(x, y);
  // cr->stroke();
  // cr->move_to(x, y);
  Gdk::Cairo::add_rectangle_to_path(cr, update_rect);
  cr->fill();

  m_DrawingArea_Scribble.queue_draw();
}

int main(int argc, char** argv)
{
  auto app = Gtk::Application::create("org.gtkmm.example");

  return app->make_window_and_run<Example_DrawingArea>(argc, argv);
}
