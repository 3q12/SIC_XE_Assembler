package exception;

public class WrongRegisterCodeException extends Exception{

    public WrongRegisterCodeException(String message) {
        super(message);
    }

    public WrongRegisterCodeException(String message, Throwable cause) {
        super(message, cause);
    }

}
