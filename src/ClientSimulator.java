import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;

public class ClientSimulator {

    public static void main(String[] args) {
        try {
            InetAddress group = InetAddress.getByName("230.0.0.1");
            int port = 4446;
            MulticastSocket socket = new MulticastSocket(port);
            socket.joinGroup(group);
            
            byte[] buffer = new byte[1024];
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
            
            System.out.println("Listening for particle updates...");
            while (true) {
                socket.receive(packet);
                String message = new String(packet.getData(), 0, packet.getLength());
                System.out.println("Received: " + message);
                // Optional: Parse and display the message
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}