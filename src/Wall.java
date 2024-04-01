import java.awt.Graphics;
import java.awt.geom.Line2D;

public class Wall {
    int startX, startY, endX, endY;

    public Wall(int startX, int startY, int endX, int endY) {
        this.startX = startX;
        this.startY = startY;
        this.endX = endX;
        this.endY = endY;
    }

    public Line2D.Double toLine2D() {
        return new Line2D.Double(startX, startY, endX, endY);
    }

    public void draw(Graphics g, DrawPanel drawPanel) {
        g.drawLine(startX, drawPanel.getHeight() - startY, endX, drawPanel.getHeight() - endY);
    }
}