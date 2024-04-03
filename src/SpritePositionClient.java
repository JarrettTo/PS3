import java.io.IOException;
import java.net.*;

public class SpritePositionClient {

    public static void main(String[] args) {
        String serverAddress = "localhost"; // Use your server's IP address if not running locally
        int serverPort = 4445; // The UDP port number your server is listening on

        try {
            DatagramSocket socket = new DatagramSocket();
            InetAddress address = InetAddress.getByName(serverAddress);

            String message = "x:100,y:200"; // Example sprite position
            byte[] buffer = message.getBytes();

            DatagramPacket packet = new DatagramPacket(buffer, buffer.length, address, serverPort);
            socket.send(packet);
            System.out.println("Sent sprite position to server.");

            socket.close();
        } catch (SocketException e) {
            System.err.println("SocketException: " + e.getMessage());
        } catch (UnknownHostException e) {
            System.err.println("UnknownHostException: " + e.getMessage());
        } catch (IOException e) {
            System.err.println("IOException: " + e.getMessage());
        }
    }
}
