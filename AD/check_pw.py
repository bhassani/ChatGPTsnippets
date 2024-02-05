#pip install ldap3
from ldap3 import Server, Connection, ALL, SIMPLE, SUBTREE

def check_ad_password(username, password):
    # Update these values with your Active Directory server details
    server_url = 'ldap://your_domain_controller'
    base_dn = 'DC=your_domain,DC=com'
    
    # Connect to the Active Directory server
    server = Server(server_url, get_info=ALL)
    connection = Connection(server, user='your_username', password='your_password', authentication=SIMPLE)

    try:
        if not connection.bind():
            raise Exception(f"Failed to bind to the server: {connection.result}")

        # Search for the user in Active Directory
        search_filter = f'(sAMAccountName={username})'
        connection.search(search_base=base_dn, search_filter=search_filter, search_scope=SUBTREE)

        if not connection.entries:
            print(f"User '{username}' not found in Active Directory.")
            return False

        # Try to authenticate the user with the provided password
        user_dn = connection.entries[0].entry_get_dn()
        auth_result = Connection(server, user=user_dn, password=password, authentication=SIMPLE).bind()

        if auth_result:
            print(f"User '{username}' authenticated successfully.")
            return True
        else:
            print(f"Authentication failed for user '{username}': {connection.result}")
            return False

    finally:
        connection.unbind()

# Example usage
username = 'your_username'
password_to_check = 'your_password'
check_ad_password(username, password_to_check)
