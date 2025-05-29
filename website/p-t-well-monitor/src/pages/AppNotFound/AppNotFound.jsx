import { useEffect } from 'react'

export default function AppNotFound(){
    useEffect(() => {
        document.title = "App Not Found";
    }, [])

    return (<>
    <h1>
        APP NOT FOUND
    </h1>
    </>);
}