import streamlit as st

st.title("Online Ticket Booking Website")

if "page" not in st.session_state:
    st.session_state.page = "p1"

if st.session_state.page == "p1":
    st.write("welcome to our online booking platform")
    st.write("to proceed further please press continue button ")
    if st.button("continue"):
        st.session_state.page="p2"

elif st.session_state.page == "p2":
    st.header("enter your Destination Details : ")
    fromStat = st.text_input("from station")
    toStat = st.text_input("To station")
    date = st.date_input("select date")
    tclass = st.selectbox("enter class : ",["Bussiness class","First class","second class","Third class"])

    if st.button("next"):
        if fromStat=="" or toStat=="":
            st.warning("Destination cant be empty")
        else:
            st.session_state.fromStat=fromStat
            st.session_state.toStat=toStat
            st.session_state.date=date
            st.session_state.tclass=tclass
            st.session_state.page="p3"
    if st.button("back"):
        st.session_state.page="p1"

elif st.session_state.page=="p3":
    st.header("Fill the Passenger details : ")

    name = st.text_input("enter name ")
    age = st.number_input("enter age")
    NOP = st.number_input("enter number of passengers")

    if st.button("CONFIRM"):
        if name=="":
            st.warning("name cant be empty")
        else:
            st.session_state.name=name
            st.session_state.age=age
            st.session_state.passenger=NOP
            st.session_state.page="p4"
    if st.button("back"):
        st.session_state.page="p2"

elif st.session_state.page=="p4":
    st.header("BOKING CONFIRMED")
    st.write("here is your details:")

    st.write("From Destination :",st.session_state.fromStat)
    st.write("To Destination :",st.session_state.toStat)
    st.write("Date :",st.session_state.date)
    st.write("Class :",st.session_state.tclass)
    st.write("name :",st.session_state.name)
    st.write("Age :",st.session_state.age)
    st.write("Number of passengers :",st.session_state.passenger)
    
    if st.button("BOOK ANOTHER "):
        st.session_state.page="p1"


