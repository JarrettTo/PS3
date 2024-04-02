import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.List;
import java.awt.geom.Point2D;
import com.google.gson.Gson; // Assuming Gson for JSON serialization

public class ClientHandler implements Runnable {
    private Socket clientSocket;
    private SimulatorState simulationState;
    private BufferedReader in;
    private PrintWriter out;
    private Gson gson = new Gson(); // For JSON serialization

    public ClientHandler(Socket clientSocket, SimulatorState simulationState) {
        this.clientSocket = clientSocket;
        this.simulationState = simulationState;
        try {
            this.in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            this.out = new PrintWriter(new OutputStreamWriter(clientSocket.getOutputStream()), true);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void run() {
        try {
            String line;
            while ((line = in.readLine()) != null) {
                // Example command processing
                if (line.startsWith("ADD PARTICLE")) {
                    // Command format: "ADD PARTICLE x,y,velocityX,velocityY"
                    String[] parts = line.split(" ")[2].split(",");
                    double x = Double.parseDouble(parts[0]);
                    double y = Double.parseDouble(parts[1]);
                    double velocityX = Double.parseDouble(parts[2]);
                    double velocityY = Double.parseDouble(parts[3]);
                    Particle particle = new Particle(x, y, velocityX, velocityY);
                    simulationState.addParticle(particle);
                } else if (line.startsWith("GET STATE")) {
                    // Send the current state to the client
                    List<Particle> particles = simulationState.getParticles();
                    String particlesJson = gson.toJson(particles);
                    out.println(particlesJson);
                }
                // Implement other commands as needed
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                clientSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
