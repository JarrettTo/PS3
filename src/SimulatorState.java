import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.CopyOnWriteArrayList;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
public class SimulatorState {
    private final List<Particle> particles;
    private final List<Wall> walls;
    private final int simulationWidth = 1280; // Assuming the size of your DrawPanel
    private final int simulationHeight = 720;
    
    // Constructor, getters, setters
    public SimulatorState(){
        particles = new CopyOnWriteArrayList<>();
        walls = new CopyOnWriteArrayList<>();
    }
    public void update(double deltaTime) {
        // Logic to update particle positions and check for collisions
        particles.forEach(particle -> {
            Point2D.Double nextPosition = particle.getNextPosition(deltaTime);
            Line2D.Double trajectory = new Line2D.Double(particle.x, particle.y, nextPosition.x, nextPosition.y);

            Point2D.Double closestCollision = null;
            Wall closestWall = null;
            double closestDistance = Double.MAX_VALUE;


            particle.checkCollisionWithBounds(simulationWidth, simulationHeight);
            for (Wall wall : walls) {
                Optional<Point2D.Double> intersectionPoint = findIntersectionPoint(trajectory, wall.toLine2D());
                if (intersectionPoint.isPresent()) {
                    double distance = intersectionPoint.get().distance(particle.x, particle.y);
                    if (distance < closestDistance) {
                        closestCollision = intersectionPoint.get();
                        closestWall = wall;
                        closestDistance = distance;
                    }
                }
            }
            if (closestCollision != null) {
                particle.handleCollision(closestCollision, closestWall);
            } else {

                particle.move(deltaTime);
            }
        });
    }
    
    // Add methods to add particles/walls, etc.
    public static Optional<Point2D.Double> findIntersectionPoint(Line2D line1, Line2D line2) {
        double x1 = line1.getX1(), y1 = line1.getY1();
        double x2 = line1.getX2(), y2 = line1.getY2();

        double x3 = line2.getX1(), y3 = line2.getY1();
        double x4 = line2.getX2(), y4 = line2.getY2();

        double den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (den == 0) return Optional.empty();

        double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den;
        double u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / den;

        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            double x = x1 + t * (x2 - x1);
            double y = y1 + t * (y2 - y1);
            return Optional.of(new Point2D.Double(x, y));
        }

        return Optional.empty();
    }
    public void addParticle(Particle particle) {
        particles.add(particle);
    }

    public void addWall(Wall wall) {
        walls.add(wall);
    }

    // Getters for particles and walls to serialize and send to clients
    public List<Particle> getParticles() {
        return particles;
    }

    public List<Wall> getWalls() {
        return walls;
    }
}
